#include "CounterFileParser.h"
#include "ProgressDialog.h"
#include "GZipFile.h"
#include "Utils.h"
#include <QtConcurrent>

CounterFileParser::CounterFileParser(QWidget *parent) :
    _parent(parent)
{
}

QString CounterFileParser::parseHeader(const QString &path, QVector<QString> &names, int &offsetFromUtc)
{
    ProgressDialog dlg(_parent);
    dlg.setDescription(QStringLiteral("Parsing counter file header..."));
    dlg.setUndeterminable();
    dlg.setCancelButtonVisible(false);

    QString error;
    auto runner = std::bind(parseHeaderInternal, std::ref(path), std::ref(offsetFromUtc), std::ref(error));

    QFutureWatcher<std::string> watcher;
    QObject::connect(&watcher, &QFutureWatcher<std::string>::finished, &dlg, &ProgressDialog::accept);
    watcher.setFuture(QtConcurrent::run(runner));

    dlg.exec();
    watcher.waitForFinished();
    if (!error.isEmpty()) {
        return error;
    }

    std::string header = watcher.result();
    const char *ptr = strchr(header.c_str() + 11, ';');
    if (ptr == nullptr) {
        error = "no counter in ";
        error += path;
        return error;
    }

    splitHeader(ptr + 1, names);
    names.last().chop(2); // Remove last "##"
    return QString();
}

QString CounterFileParser::parseData(const QString &path, const IndexNameMap &inm, CounterDataMap &cdm)
{
    volatile bool working = true;

    ProgressDialog dlg(_parent);
    dlg.setDescription(QStringLiteral("Parsing counter file data..."));
    dlg.setRange(0, 100);
    QObject::connect(&dlg, &ProgressDialog::canceling, [&working]() { working = false; });

    QFutureWatcher<QString> watcher;
    QObject::connect(&watcher, &QFutureWatcher<QString>::finished, &dlg, &ProgressDialog::accept);
    auto runner = std::bind(parseDataInternal, std::cref(path), std::cref(inm), std::cref(working), std::ref(dlg), std::ref(cdm));
    watcher.setFuture(QtConcurrent::run(runner));

    dlg.exec();
    watcher.waitForFinished();
    return working ? watcher.result() : QStringLiteral("canceled");
}

std::string CounterFileParser::parseHeaderInternal(const QString &path, int &offsetFromUtc, QString &error)
{
    GZipFile fileReader;
    if (!fileReader.open(path, GZipFile::ReadOnly)) {
        error = "failed to open ";
        error += path;
        return std::string();
    }

    std::string header;
    if (!fileReader.readLine(header)) {
        error = "failed to read ";
        error += path;
        return std::string();
    }

    if (strncmp(header.c_str(), "##date;time", 11) ||
        strcmp(header.c_str() + header.length() - 2, "##"))
    {
        error = "invalid header format in ";
        error += path;
        return std::string();
    }

    if (header[11] != ';') {
        offsetFromUtc = atoi(header.c_str() + 11);
    } else {
        offsetFromUtc = std::numeric_limits<int>::max();
    }

    return header;
}

static inline const char *searchr(const char *ptr, unsigned int len, char ch, const char **newline)
{
    for (unsigned int i = 0; i < len; ++i) {
        if (ptr[i] == ch) {
            *newline = nullptr;
            return ptr + i;
        }
        if (ptr[i] == '\n') {
            *newline = ptr + i;
            return nullptr;
        }
    }
    return *newline = nullptr;
}

QString CounterFileParser::parseDataInternal(const QString &path, const IndexNameMap &inm, const volatile bool &working, ProgressDialog &dlg, CounterDataMap &cdm)
{
    GZipFile reader;
    if (!reader.open(path, GZipFile::ReadOnly)) {
        QString error("failed to open ");
        error += path;
        return error;
    }

    // Consume the header
    std::string line;
    if (!reader.readLine(line)) {
        QString error("failed to read header from ");
        error += path;
        return error;
    }

    const int BUFFER_SIZE = 4096;
    const char *ptr = nullptr;
    int progress = 0, parsed = 0, copied = 0, index = 2;
    QCPGraphData data;
    QList<int> indexes = inm.keys();
    QScopedPointer<char, QScopedPointerArrayDeleter<char>> buffer(new char[BUFFER_SIZE]);

    int bytes = reader.read(buffer.data(), BUFFER_SIZE);
    if (bytes <= 0) {
        QString error("failed to read data from ");
        error += path;
        return error;
    }

    int len = bytes;
    const char *newline = buffer.data();
    while (working) {
        int newProgress = reader.progress();
        if (newProgress > progress) {
            QMetaObject::invokeMethod(&dlg, "setValue", Qt::QueuedConnection, Q_ARG(int, newProgress));
            progress = newProgress;
        }
        if (newline) {
            QDateTime dt = QDateTime::fromString(QString::fromLatin1(newline, DTFMT_IN_CSV_LEN), DTFMT_IN_CSV);
            if (dt.isValid()) {
                index = 2;
                parsed = 0;
                data.key = dt.toSecsSinceEpoch();
                len -= DTFMT_IN_CSV_LEN + 1;
                ptr = newline + DTFMT_IN_CSV_LEN + 1;
            } else {
                QString error("invalid date time format ");
                error += QLatin1String(newline, DTFMT_IN_CSV_LEN);
                return error;
            }
        }
        char *suspectChar;
        const char *semicolon;
        while ((semicolon = searchr(ptr, len, ';', &newline)) != nullptr) {
            if (index == indexes.at(parsed)) {
                if (*ptr != ';') {
                    data.value = strtod(ptr, &suspectChar);
                } else {
                    data.value = NAN;
                }
                CounterData &cdata = cdm[inm.value(index)];
                cdata.data->add(data);
                // Must check data.value firstly since the suspectChar is updated only when it is NOT a NaN.
                if (!qIsNaN(data.value) && *suspectChar == 's') {
                    cdata.suspectKeys.insert(data.key);
                }
                if (++parsed == indexes.size()) {
                    len -= semicolon - ptr;
                    ptr = semicolon;
                    while ((newline = (const char *)memchr(ptr, '\n', len)) == nullptr) {
                        if ((bytes = reader.read(buffer.data(), BUFFER_SIZE)) <= 0) {
                            if (reader.eof()) {
                                return QString();
                            }
                            return QString(reader.error());
                        }
                        newProgress = reader.progress();
                        if (newProgress > progress) {
                            QMetaObject::invokeMethod(&dlg, "setValue", Qt::QueuedConnection, Q_ARG(int, newProgress));
                            progress = newProgress;
                        }
                        len = bytes;
                        ptr = buffer.data();
                    }
                    break;
                }
            }
            ++index;
            len -= (semicolon - ptr) + 1;
            ptr = semicolon + 1;
        }
        if (newline) {
            if (parsed < indexes.size() && index == indexes.at(parsed)) {
                if (*ptr != ';') {
                    data.value = strtod(ptr, &suspectChar);
                } else {
                    data.value = NAN;
                }
                CounterData &cdata = cdm[inm.value(index)];
                cdata.data->add(data);
                // Must check data.value firstly since the suspectChar is updated only when it is NOT a NaN.
                if (!qIsNaN(data.value) && *suspectChar == 's') {
                    cdata.suspectKeys.insert(data.key);
                }
            }
            len -= newline - ptr;
            ptr = newline;
            if (len > DTFMT_IN_CSV_LEN + 1) {
                ++newline;
                --len;
                continue;
            } else {
                copied = len - 1;
                memcpy(buffer.data(), newline + 1, copied);
                newline = buffer.data();
            }
        } else {
            copied = len;
            memcpy(buffer.data(), ptr, copied);
            ptr = buffer.data();
        }
        if ((bytes = reader.read(buffer.data() + copied, BUFFER_SIZE - copied)) <= 0) {
            if (reader.eof()) {
                return QString();
            }
            return QString(reader.error());
        }
        len = copied + bytes;
    }

    return QString();
}

void CounterFileParser::splitHeader(const char *str, QVector<QString> &out)
{
    const char *ptr;
    while ((ptr = strchr(str, ';')) != nullptr) {
        out.append(QLatin1String(str, ptr - str));
        str = ptr + 1;
    }
    if (*str) {
        out.append(QString(str));
    }
}
