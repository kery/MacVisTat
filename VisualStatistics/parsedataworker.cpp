#include "parsedataworker.h"

#define PARSE_OK 0
#define PARSE_CANCELED 1
#define PARSE_FAILED 2

#include <QThread>
#include <QDebug>

ParseDataWorker::ParseDataWorker(QObject *parent) :
    QObject(parent),
    _canceled(false)
{
}

ParseDataWorker::~ParseDataWorker()
{
}

void ParseDataWorker::parseData(const ParseDataParam &param)
{
    // Calculate the progress range
    emit progressLabelUpdated("Calculating execution time...");
    int lineCount = 0;
    for (auto iter = param.fileInfo.begin(); iter != param.fileInfo.end(); ++iter) {
        // If iter.value() equals 0 it means that the corresponding statistics file's line
        // count is unknown (user didn't check it while showing the time duration), in this
        // condition we can consider it has one line
        lineCount += qMax(1, iter.value());
    }
    emit progressRangeWorkedOut(0, lineCount);

    ParsedResult result;
    // Create QCPDataMap object for each statistics name
    for (const QString &statName : param.statNames) {
        result.data.insert(statName, new QCPDataMap());
    }

    bool interrupted = false;
    int progress = 0;
    int keyVal = 0;
    for (auto iter = param.fileInfo.begin(); iter != param.fileInfo.end(); ++iter) {
        QFileInfo fileInfo(iter.key());
        emit progressLabelUpdated("Parsing " + fileInfo.fileName());
        int ret = parseFile(iter.key(), iter.value() > 0, progress, keyVal, result);
        if (ret != PARSE_OK) {
            interrupted = true;
            if (ret != PARSE_CANCELED) {
                emit progressLabelUpdated(QString("Parse %1 <font color='red'>failed!</font>").arg(iter.key()));
            }
            break;
        }
    }

    if (interrupted) {
        for (auto iter = result.data.begin(); iter != result.data.end(); ++iter) {
            // If failed delete the allocated QCPDataMap object, otherwise QCPGraph object will take the ownership
            delete iter.value();
        }
    } else {
        emit dataReady(result, param.multipleWindows);
    }
}

int ParseDataWorker::parseFile(const QString &file, bool lineCountKnown, int &progress, int &keyVal, ParsedResult &result)
{
    GZipFile gzFile(file);
    QMap<QString, int> indexes; // Key is statistics name, value is the index in line
    for (auto iter = result.data.begin(); iter != result.data.end(); ++iter) {
        indexes.insert(iter.key(), -1);
    }
    if (!calcStatNamesIndex(gzFile, indexes)) {
        return PARSE_FAILED;
    }

    // Since header has been parsed so update the progress if necessary
    // Note: if line count is known step progress by line, otherwise step by file
    if (lineCountKnown) {
        emit progressValueUpdated(++progress);
    }

    // Check if all statistics name's index are valid
    for (auto iter = indexes.begin(); iter != indexes.end(); ++iter) {
        if (iter.value() < 0) {
            return PARSE_FAILED;
        }
    }

    QString line;
    QCPData data;
    while (!_canceled && gzFile.readLine(line)) {
        if (line.isEmpty()) {
            if (lineCountKnown) {
                emit progressValueUpdated(++progress);
            }
            continue;
        }
        QVector<QStringRef> refs = line.splitRef(';');
        QDateTime dt = QDateTime::fromString(line.left(19), "dd.MM.yyyy;HH:mm:ss");
        if (!dt.isValid()) {
            return PARSE_FAILED;
        }
        result.dateTimes << dt.toTime_t();
        // Don't use time_t as key because some statistics may have large time gap
        // So use increased integer as key and set the time as the axis label
        data.key = keyVal++;
        for (auto iter = indexes.begin(); iter != indexes.end(); ++iter) {
            if (iter.value() >= refs.size()) {
                return PARSE_FAILED;
            }
            data.value = refs.at(iter.value()).toInt();
            result.data.value(iter.key())->insert(data.key, data);
        }
        if (lineCountKnown) {
            emit progressValueUpdated(++progress);
        }
    }

    if (_canceled) {
        return PARSE_CANCELED;
    }

    if (!lineCountKnown) {
        emit progressValueUpdated(++progress);
    }

    return PARSE_OK;
}

bool ParseDataWorker::calcStatNamesIndex(GZipFile &gzFile, QMap<QString, int> &indexes)
{
    QString header;
    if (!gzFile.readLine(header)) {
        return false;
    }

    QStringList strList = header.split(';');
    for (int i = 0; i < strList.size(); ++i) {
        if (indexes.contains(strList.at(i))) {
            indexes[strList.at(i)] = i;
        }
    }
    return true;
}
