#include "statisticsfileparser.h"
#include "progressdialog.h"
#include "gzipfile.h"
#include "utils.h"
#include <QtConcurrent>
#include <functional>

using Result = StatisticsFileParser::Result;

StatisticsFileParser::StatisticsFileParser(ProgressDialog &dialog) :
    m_dialog(dialog)
{
}

static inline const char * searchr(const char *ptr, unsigned int len, char ch, const char **newline)
{
    for (unsigned int i = 0; i < len; ++i) {
        if (ptr[i] == ch) {
            *newline = NULL;
            return ptr + i;
        }
        if (ptr[i] == '\n') {
            *newline = ptr + i;
            return NULL;
        }
    }
    return *newline = NULL;
}

Result mappedFunction(const StatisticsFileParser::IndexNameMap &inm,
                      ProgressDialog &dialog,
                      volatile bool &working,
                      const QString &fileName)
{
    Result result;
    std::string line;
    GZipFile gzFile(fileName);

    // Read the header
    if (!gzFile.readLine(line)) {
        result.failedFiles << QFileInfo(fileName).fileName();
        return result;
    }

    QCPData data;
    int progress = 0;
    Statistics::NameDataMap &ndm = result.nndm[
            StatisticsFileParser::getNodeFromFileName(QFileInfo(fileName).fileName())];
    QList<int> indexes = inm.keys();

    const unsigned int BUFFER_SIZE = 4096;
    unsigned int index, parsed, copied, len;
    std::unique_ptr<char[]> buffer(new char[BUFFER_SIZE]);
    const char *newline, *ptr, *semicolon;

    int bytes = gzFile.read(buffer.get(), BUFFER_SIZE);
    if (bytes <= 0) {
        result.failedFiles << QFileInfo(fileName).fileName();
        return result;
    }

    len = bytes;
    newline = buffer.get();
    while (working) {
        int newProgress = gzFile.progress();
        if (newProgress - progress >= 9) {
            QMetaObject::invokeMethod(&dialog, "increaseValue", Qt::QueuedConnection,
                Q_ARG(int, newProgress - progress));
            progress = newProgress;
        }
        if (newline) {
            QDateTime dt = QDateTime::fromString(QString::fromLatin1(newline,
                DT_FORMAT_IN_FILE.length()), DT_FORMAT_IN_FILE);
            if (dt.isValid()) {
                index = 2;
                parsed = 0;
                data.key = dt.toTime_t();
                len -= DT_FORMAT_IN_FILE.length() + 1;
                ptr = newline + DT_FORMAT_IN_FILE.length() + 1;
            } else {
                result.failedFiles << QFileInfo(fileName).fileName();
                return result;
            }
        }
        while (semicolon = searchr(ptr, len, ';', &newline)) {
            if (index == indexes.at(parsed)) {
                data.value = strtoll(ptr, NULL, 10);
                ndm[inm.value(index)].insert(data.key, data);
                if (++parsed == indexes.size()) {
                    len -= semicolon - ptr;
                    ptr = semicolon;
                    while ((newline = (const char *)memchr(ptr, '\n', len)) == NULL) {
                        if ((bytes = gzFile.read(buffer.get(), BUFFER_SIZE)) <= 0) {
                            return result;
                        }
                        len = bytes;
                        ptr = buffer.get();
                    }
                    break;
                }
            }
            ++index;
            len -= (semicolon - ptr) + 1;
            ptr = semicolon + 1;
        }
        if (newline) {
            if (parsed < (unsigned int)indexes.size() && index == indexes.at(parsed)) {
                data.value = strtoll(ptr, NULL, 10);
                ndm[inm.value(index)].insert(data.key, data);
            }
            len -= newline - ptr;
            ptr = newline;
            if (len > (unsigned int)DT_FORMAT_IN_FILE.length() + 1) {
                ++newline;
                --len;
                continue;
            } else {
                copied = len - 1;
                memcpy(buffer.get(), newline + 1, copied);
                newline = buffer.get();
            }
        } else {
            copied = len;
            memcpy(buffer.get(), ptr, copied);
            ptr = buffer.get();
        }
        if ((bytes = gzFile.read(buffer.get() + copied, BUFFER_SIZE - copied)) <= 0) {
            return result;
        }
        len = copied + bytes;
    }
    return result;
}

void reducedFunction(Result &result, const Result &partial)
{
    if (partial.failedFiles.isEmpty()) {
        const QString &node = partial.nndm.firstKey();
        const Statistics::NameDataMap &ndm = partial.nndm.first();
        Statistics::NameDataMap &finalNdm = result.nndm[node];

        for (auto iter = ndm.begin(); iter != ndm.end(); ++iter) {
            const QCPDataMap &src = iter.value();
            QCPDataMap &dest = finalNdm[iter.key()];
            for (auto innerIter = src.begin(); innerIter != src.end(); ++innerIter) {
                dest.insert(innerIter.key(), innerIter.value());
            }
        }
    } else {
        for (const QString &file : partial.failedFiles) {
            result.failedFiles << file;
        }
    }
}

bool StatisticsFileParser::parseFileData(const IndexNameMap &inm,
                                         const QVector<QString> &fileNames,
                                         Result &result)
{
    volatile bool working = true;
    // A lambda capture variable must be from an enclosing function scope
    ProgressDialog &dialog = m_dialog;
    auto mappedCallable = std::bind(mappedFunction,
                                    std::cref(inm),
                                    std::ref(dialog),
                                    std::ref(working),
                                    std::placeholders::_1);

    dialog.setRange(0, fileNames.size() * 100);

    // We don't use wathcer to monitor progress because it base on item count in the container, this
    // is not accurate. Instead, we calculate the progress ourselves
    QFutureWatcher<Result> watcher;
    QObject::connect(&watcher, SIGNAL(finished()), &dialog, SLOT(accept()));
    QObject::connect(&dialog, &ProgressDialog::canceled, [&working, &watcher] () {watcher.cancel();working = false;});
    watcher.setFuture(QtConcurrent::mappedReduced<Result>(fileNames, mappedCallable, reducedFunction));
    dialog.exec();
    watcher.waitForFinished();

    if (watcher.isCanceled()) {
        return false;
    } else {
        // Result type has synthesized move assignment operator
        result = watcher.result();
        return true;
    }
}

std::string parseHeader(QStringList &filePaths, QStringList &failInfo, ProgressDialog &dialog)
{
    std::string result;
    int progress = 0;

    auto iter = filePaths.begin();
    for (; iter != filePaths.end(); ++iter) {
        QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection, Q_ARG(int, ++progress));
        if (GZipFile(*iter).readLine(result)) {
            if (strncmp(result.c_str(), "##date;time;", 12) == 0 &&
                strcmp(result.c_str() + result.length() - 2, "##") == 0)
            {
                break;
            } else {
                result.clear();
                failInfo.append(QStringLiteral("%1: invalid header format!").arg(QFileInfo(*iter).fileName()));
            }
        } else {
            failInfo.append(QStringLiteral("%1: reading header failed!").arg(QFileInfo(*iter).fileName()));
        }
        iter->clear(); // Clear the file path so that we can remove it from filePaths later
    }

    if (iter != filePaths.end()) {
        // Parse the rest files' header
        std::string header;
        while (++iter != filePaths.end()) {
            QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection, Q_ARG(int, ++progress));
            if (GZipFile(*iter).readLine(header)) {
                if (header != result) {
                    failInfo.append(QStringLiteral("%1: header is not the same!").arg(QFileInfo(*iter).fileName()));
                    iter->clear();
                }
            } else {
                failInfo.append(QStringLiteral("%1: reading header failed!").arg(QFileInfo(*iter).fileName()));
                iter->clear();
            }
        }
    }

    // Remove the failed file
    auto newEnd = std::remove_if(filePaths.begin(), filePaths.end(), [] (const QString &filePath) {
        return filePath.isEmpty();
    });
    filePaths.erase(newEnd, filePaths.end());

    QMetaObject::invokeMethod(&dialog, "accept", Qt::QueuedConnection);
    return result;
}

std::string StatisticsFileParser::parseFileHeader(QStringList &filePaths, QStringList &failInfo)
{
    m_dialog.setRange(0, filePaths.size());
    QFuture<std::string> future = QtConcurrent::run(
                std::bind(parseHeader, std::ref(filePaths), std::ref(failInfo), std::ref(m_dialog)));

    m_dialog.exec();
    future.waitForFinished();
    return future.result();
}

void checkHeader(QStringList &filePaths, QStringList &failInfo, ProgressDialog &dialog)
{
    int progress = 0;

    QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection, Q_ARG(int, ++progress));
    std::string header;
    if (GZipFile(filePaths.at(0)).readLine(header)) {
        std::string tempHeader;
        for (auto iter = filePaths.begin() + 1; iter != filePaths.end(); ++iter) {
            QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection, Q_ARG(int, ++progress));
            if (GZipFile(*iter).readLine(tempHeader)) {
                if (tempHeader != header) {
                    failInfo.append(QStringLiteral("%1: header is not the same!").arg(QFileInfo(*iter).fileName()));
                    iter->clear();
                }
            } else {
                failInfo.append(QStringLiteral("%1: reading header failed!").arg(QFileInfo(*iter).fileName()));
                iter->clear();
            }
        }
    } else {
        failInfo.append(QStringLiteral("%1 reading header failed, stop checking!").arg(QFileInfo(filePaths.at(0)).fileName()));
        filePaths.erase(filePaths.begin() + 1, filePaths.end());
    }

    // Remove the failed file
    auto newEnd = std::remove_if(filePaths.begin() + 1, filePaths.end(), [] (const QString &filePath) {
        return filePath.isEmpty();
    });
    filePaths.erase(newEnd, filePaths.end());

    QMetaObject::invokeMethod(&dialog, "accept", Qt::QueuedConnection);
}

void StatisticsFileParser::checkFileHeader(QStringList &filePaths, QStringList &failInfo)
{
    m_dialog.setRange(0, filePaths.size());
    QFuture<void> future = QtConcurrent::run(
                std::bind(checkHeader, std::ref(filePaths), std::ref(failInfo), std::ref(m_dialog)));

    m_dialog.exec();
    future.waitForFinished();
}

QString parseTimeDurationFunc(const QString &filePath, volatile bool &working, ProgressDialog &dialog)
{
    const int TIME_STR_LEN = 19;

    QString result(filePath);
    GZipFile gzFile(filePath);
    QRegExp regExp(QStringLiteral("^\\d{2}\\.\\d{2}\\.\\d{4};\\d{2}:\\d{2}:\\d{2}$"));
    std::string firstLine, lastLine;

    // Ignore the first line because it is header
    if (!gzFile.readLine(firstLine) || !gzFile.readLine(firstLine)) {
        result += QStringLiteral("##Time duration: parse failed: %1").arg(QFileInfo(filePath).fileName());
        return result;
    }

    // Check format
    if (!regExp.exactMatch(QString::fromStdString(firstLine.substr(0, TIME_STR_LEN)))) {
        result += QStringLiteral("##Time duration: invalid format: %1").arg(QFileInfo(filePath).fileName());
        return result;
    }

    // Get the last line
    int progress = 0;
    while (working && gzFile.readLine(lastLine)) {
        int newProgress = gzFile.progress();
        if (newProgress - progress > 10) {
            QMetaObject::invokeMethod(&dialog, "increaseValue", Qt::QueuedConnection,
                                      Q_ARG(int, newProgress - progress));
            progress = newProgress;
        }
    }

    // Check format
    if (!regExp.exactMatch(QString::fromStdString(lastLine.substr(0, TIME_STR_LEN)))) {
        result += QStringLiteral("##Time duration: invalid format: %1").arg(QFileInfo(filePath).fileName());
        return result;
    }

    result += QStringLiteral("##Time duration: %1 - %2").
            arg(QString::fromStdString(firstLine.substr(0, TIME_STR_LEN)).replace(';', ' ')).
            arg(QString::fromStdString(lastLine.substr(0, TIME_STR_LEN)).replace(';', ' '));
    return result;
}

void StatisticsFileParser::parseTimeDuration(const QStringList &filePaths, QObject *receiver, const char *slot)
{
    volatile bool working = true;
    m_dialog.setRange(0, filePaths.size() * 100);

    auto watcher = new QFutureWatcher<QString>();
    QObject::connect(watcher, SIGNAL(resultReadyAt(int)), receiver, slot);
    QObject::connect(watcher, SIGNAL(finished()), &m_dialog, SLOT(accept()));
    QObject::connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
    QObject::connect(&m_dialog, &ProgressDialog::canceled, [&working, watcher] () {
        watcher->cancel();
        working = false;
    });

    watcher->setFuture(QtConcurrent::mapped(filePaths, std::bind(parseTimeDurationFunc,
                                                                 std::placeholders::_1,
                                                                 std::ref(working),
                                                                 std::ref(m_dialog))));
    m_dialog.exec();
    watcher->waitForFinished();
}

QString StatisticsFileParser::getNodeFromFileName(const QString &fileName)
{
    int pos = fileName.indexOf(QStringLiteral("__"));
    Q_ASSERT(pos > 0);
    return fileName.left(pos);
}
