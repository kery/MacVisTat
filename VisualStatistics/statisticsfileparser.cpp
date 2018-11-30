#include "statisticsfileparser.h"
#include "progressdialog.h"
#include "gzipfile.h"
#include "utils.h"
#include <QtConcurrent>
#include <functional>
#include <set>
#include <memory>

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

struct FileDataResult
{
    Statistics::NodeNameDataMap nndm;
    QVector<QString> errors;
};

static FileDataResult doParseFileData(const StatisticsFileParser::IndexNameMap &inm,
                      ProgressDialog &dialog,
                      volatile bool &working,
                      const QString &fileName)
{
    FileDataResult result;
    std::string line;
    GzipFile reader;

    // Read the header
    if (!reader.open(fileName)) {
        result.errors.reserve(1);
        result.errors.append("failed to open " + fileName);
        return result;
    }

    if (!reader.readLine(line)) {
        result.errors.reserve(1);
        result.errors.append("failed to read header of " + fileName);
        return result;
    }

    QCPData data;
    int progress = 0;
    Statistics::NameDataMap &ndm = result.nndm[
            StatisticsFileParser::getNodeFromFileName(QFileInfo(fileName).fileName())];
    QList<int> indexes = inm.keys();

    const int BUFFER_SIZE = 4096;
    int index = 2;
    int parsed = 0;
    int copied = 0;
    std::unique_ptr<char[]> buffer(new char[BUFFER_SIZE]);
    const char *ptr = nullptr;

    int bytes = reader.read(buffer.get(), BUFFER_SIZE);
    if (bytes <= 0) {
        result.errors.reserve(1);
        result.errors.append("failed to read data from " + fileName);
        return result;
    }

    int len = bytes;
    const char *newline = buffer.get();
    while (working) {
        int newProgress = reader.progress() / 10;
        if (newProgress > progress) {
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
                result.errors.reserve(1);
                result.errors.append("invalid date time format in " + fileName);
                return result;
            }
        }
        const char *semicolon;
        while ((semicolon = searchr(ptr, len, ';', &newline)) != nullptr) {
            if (index == indexes.at(parsed)) {
                data.value = strtod(ptr, NULL);
                ndm[inm.value(index)].insert(data.key, data);
                if (++parsed == indexes.size()) {
                    len -= semicolon - ptr;
                    ptr = semicolon;
                    while ((newline = (const char *)memchr(ptr, '\n', len)) == NULL) {
                        if ((bytes = reader.read(buffer.get(), BUFFER_SIZE)) <= 0) {
                            return result;
                        }
                        newProgress = reader.progress() / 10;
                        if (newProgress > progress) {
                            QMetaObject::invokeMethod(&dialog, "increaseValue", Qt::QueuedConnection,
                                Q_ARG(int, newProgress - progress));
                            progress = newProgress;
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
            if (parsed < indexes.size() && index == indexes.at(parsed)) {
                data.value = strtod(ptr, NULL);
                ndm[inm.value(index)].insert(data.key, data);
            }
            len -= newline - ptr;
            ptr = newline;
            if (len > DT_FORMAT_IN_FILE.length() + 1) {
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
        if ((bytes = reader.read(buffer.get() + copied, BUFFER_SIZE - copied)) <= 0) {
            return result;
        }
        len = copied + bytes;
    }
    return result;
}

static void doMergeFileData(FileDataResult &finalResult, const FileDataResult &intermediaResult)
{
    if (intermediaResult.errors.isEmpty()) {
        const QString &node = intermediaResult.nndm.firstKey();
        const Statistics::NameDataMap &ndm = intermediaResult.nndm.first();
        Statistics::NameDataMap &finalNdm = finalResult.nndm[node];

        for (auto iter = ndm.begin(); iter != ndm.end(); ++iter) {
            const QCPDataMap &src = iter.value();
            QCPDataMap &dest = finalNdm[iter.key()];
            for (auto innerIter = src.begin(); innerIter != src.end(); ++innerIter) {
                dest.insert(innerIter.key(), innerIter.value());
            }
        }
    } else {
        finalResult.errors.append(intermediaResult.errors);
    }
}

bool StatisticsFileParser::parseFileData(const IndexNameMap &inm, const QVector<QString> &fileNames,
    Statistics::NodeNameDataMap &nndm, QVector<QString> &errors)
{
    volatile bool working = true;
    // A lambda capture variable must be from an enclosing function scope
    ProgressDialog &dialog = m_dialog;
    auto mappedFunction = std::bind(doParseFileData,
                                    std::cref(inm),
                                    std::ref(dialog),
                                    std::ref(working),
                                    std::placeholders::_1);

    dialog.setRange(0, fileNames.size() * 10);
    dialog.setValue(0);

    // We don't use wathcer to monitor progress because it base on item count in the container, this
    // is not accurate. Instead, we calculate the progress ourselves
    QFutureWatcher<FileDataResult> watcher;
    QObject::connect(&watcher, SIGNAL(finished()), &dialog, SLOT(accept()));
    QObject::connect(&dialog, &ProgressDialog::canceling, [&working, &watcher] () {
        working = false;
        watcher.cancel();
    });
    watcher.setFuture(QtConcurrent::mappedReduced<FileDataResult>(fileNames, mappedFunction, doMergeFileData));
    dialog.exec();

    if (watcher.isCanceled()) {
        return false;
    } else {
        FileDataResult result = watcher.result();
        nndm.swap(result.nndm);
        errors.swap(result.errors);
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
        
        GzipFile reader;
        if (reader.open(*iter) && reader.readLine(result)) {
            if (strncmp(result.c_str(), "##date;time;", 12) == 0 &&
                strcmp(result.c_str() + result.length() - 2, "##") == 0)
            {
                break;
            } else {
                result.clear();
                failInfo.append("invalid header format of " + *iter);
            }
        } else {
            failInfo.append("failed to read header from " + *iter);
        }
        iter->clear(); // Clear the file path so that we can remove it from filePaths later
    }

    if (iter != filePaths.end()) {
        // Parse the rest files' header
        std::string header;
        while (++iter != filePaths.end()) {
            QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection, Q_ARG(int, ++progress));

            GzipFile reader;
            if (reader.open(*iter) && reader.readLine(header)) {
                if (header != result) {
                    failInfo.append("incompatible header of " + *iter);
                    iter->clear();
                }
            } else {
                failInfo.append("failed to read header from " + *iter);
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
    m_dialog.setValue(0);
    QFuture<std::string> future = QtConcurrent::run(
                std::bind(parseHeader, std::ref(filePaths), std::ref(failInfo), std::ref(m_dialog)));

    m_dialog.exec();
    return future.result();
}

static void checkHeader(QStringList &filePaths, QStringList &failInfo, ProgressDialog &dialog)
{
    int progress = 0;

    QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection, Q_ARG(int, ++progress));
    
    std::string header;
    GzipFile reader;
    if (reader.open(filePaths.at(0)) && reader.readLine(header)) {
        std::string tempHeader;
        for (auto iter = filePaths.begin() + 1; iter != filePaths.end(); ++iter) {
            QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection, Q_ARG(int, ++progress));

            GzipFile reader;
            if (reader.open(*iter) && reader.readLine(tempHeader)) {
                if (tempHeader != header) {
                    failInfo.append("incompatible header of " + *iter);
                    iter->clear();
                }
            } else {
                failInfo.append("failed to read header from " + *iter);
                iter->clear();
            }
        }
    } else {
        failInfo.append("failed to read header from " + filePaths[0]);
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
    m_dialog.setValue(0);
    QFuture<void> future = QtConcurrent::run(
                std::bind(checkHeader, std::ref(filePaths), std::ref(failInfo), std::ref(m_dialog)));

    m_dialog.exec();
    future.waitForFinished();
}

struct KciKpiNameNode
{
    QString name;
    mutable std::set<KciKpiNameNode> children;

    KciKpiNameNode();
    KciKpiNameNode(const QStringRef &n);
    KciKpiNameNode(const QString &n);

    bool operator<(const KciKpiNameNode &other) const;
};

KciKpiNameNode::KciKpiNameNode()
{
}

KciKpiNameNode::KciKpiNameNode(const QStringRef &n) :
    name(n.toString())
{
}

KciKpiNameNode::KciKpiNameNode(const QString &n) :
    name(n)
{
}

bool KciKpiNameNode::operator<(const KciKpiNameNode &other) const
{
    return name < other.name;
}

static void mergeKciKpiNameNode(const KciKpiNameNode &dest, const KciKpiNameNode &src)
{
    for (const KciKpiNameNode &child : src.children) {
        auto insertResult = dest.children.insert(child);
        mergeKciKpiNameNode(*insertResult.first, child);
    }
}

static void feedFullKciKpiNames(const KciKpiNameNode &node, QVector<QString> &result, QVector<const QString *> &stack)
{
    if (node.children.empty()) {
        QString fullName;
        for (const QString *item : stack) {
            fullName.append(*item);
            fullName.append(',');
        }
        fullName.append(node.name);
        result.append(std::move(fullName));
    } else {
        stack.push_back(&node.name);
        for (const KciKpiNameNode &child : node.children) {
            feedFullKciKpiNames(child, result, stack);
        }
        stack.pop_back();
    }
}

static QVector<QString> genFullKciKpiNames(const KciKpiNameNode &root)
{
    QVector<QString> result;
    for (const KciKpiNameNode &child : root.children) {
        QVector<const QString *> stack;
        feedFullKciKpiNames(child, result, stack);
    }
    return result;
}

struct XmlHeaderResult
{
    KciKpiNameNode root;
    QVector<QString> errors;
};

static XmlHeaderResult parseXmlHeader(volatile const bool &working, const QString &filePath)
{
    XmlHeaderResult result;

    GzipFile fileReader;
    if (!fileReader.open(filePath)) {
        result.errors.reserve(1);
        result.errors.append("failed to open " + filePath);
        return result;
    }

    bool isMeasTypeElement = false;
    QVector<QString> measTypes;
    QXmlStreamReader xmlReader(&fileReader);

    while (working && !xmlReader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = xmlReader.readNext();
        if (tokenType == QXmlStreamReader::StartElement) {
            if (xmlReader.name() == "measType") {
                isMeasTypeElement = true;
            } else if (xmlReader.name() == "measValue") {
                QXmlStreamAttributes attributes = xmlReader.attributes();
                if (attributes.hasAttribute(QLatin1String("measObjLdn"))) {
                    const KciKpiNameNode *nameNode = &result.root;
                    QVector<QStringRef> objLdnItems = attributes.value(QLatin1String("measObjLdn")).split(',');
                    for (const QStringRef &ldnItem : objLdnItems) {
                        auto insertResult = nameNode->children.insert(ldnItem);
                        nameNode = &(*insertResult.first);
                    }
                    for (const QString &measType : measTypes) {
                        nameNode->children.insert(measType);
                    }
                } else {
                    result.errors.reserve(1);
                    result.errors.append("measObjLdn is missing in file " + filePath);
                    return result;
                }
            }
        } else if (tokenType == QXmlStreamReader::EndElement) {
            if (xmlReader.name() == "measType") {
                isMeasTypeElement = false;
            } else if (xmlReader.name() == "measInfo") {
                measTypes.clear();
            }
        } else if (tokenType == QXmlStreamReader::Characters) {
            if (isMeasTypeElement) {
                measTypes.append(xmlReader.text().toString());
            }
        } else if (tokenType == QXmlStreamReader::Invalid) {
            result.errors.reserve(1);
            result.errors.append("failed to parse KCI/KPI file " + filePath + ": " + xmlReader.errorString());
            return result;
        }
    }

    return result;
}

static void mergeXmlHeader(XmlHeaderResult &finalResult, const XmlHeaderResult &intermediateResult)
{
    if (intermediateResult.errors.isEmpty()) {
        mergeKciKpiNameNode(finalResult.root, intermediateResult.root);
    } else {
        finalResult.errors.append(intermediateResult.errors);
    }
}

static void writeXmlHeader(ProgressDialog &dialog, volatile const bool &working,
    GzipFile &fileWriter, const KciKpiNameNode &root, QHash<QString, int> &indexes)
{
    int progress = 0;
    QVector<QString> fullNames = genFullKciKpiNames(root);

    fileWriter.write("##date;time", 11);

    for (int index = 0; working && index < fullNames.size(); ++index) {
        const QString &fullName = fullNames[index];
        indexes[fullName] = index;
        fileWriter.write(";", 1);
        fileWriter.write(fullName);

        int newProgress = (static_cast<float>(index) / fullNames.size()) * 100;
        if (newProgress > progress) {
            progress = newProgress;
            QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection, Q_ARG(int, progress));
        }
    }

    fileWriter.write("##\n", 3);
}

struct XmlDataResult
{
    QString dateTime;
    QVector<QString> values;
    QVector<QString> errors;
};

static XmlDataResult parseXmlData(const QString &filePath, const QHash<QString, int> &indexes,
    volatile const bool &working)
{
    XmlDataResult result;

    GzipFile fileReader;
    if (!fileReader.open(filePath)) {
        result.errors.reserve(1);
        result.errors.append("failed to open " + filePath);
        return result;
    }

    bool isMeasTypeElement = false;
    bool isRElement = false;
    QString valueP, objLdn;
    QHash<QString, QString> pMeasType;
    QXmlStreamReader xmlReader(&fileReader);
    QVector<QString> values(indexes.size(), QStringLiteral("0"));

    while (working && !xmlReader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = xmlReader.readNext();
        if (tokenType == QXmlStreamReader::StartElement) {
            if (xmlReader.name() == "measType") {
                isMeasTypeElement = true;

                QXmlStreamAttributes attributes = xmlReader.attributes();
                if (!attributes.hasAttribute(QLatin1String("p"))) {
                    result.errors.reserve(1);
                    result.errors.append("no 'p' attribute of measType in file " + filePath);
                    return result;
                }
                valueP = attributes.value(QLatin1String("p")).toString();
            } else if (xmlReader.name() == "measValue") {
                QXmlStreamAttributes attributes = xmlReader.attributes();
                objLdn = attributes.value(QLatin1String("measObjLdn")).toString();
            } else if (xmlReader.name() == 'r') {
                isRElement = true;

                QXmlStreamAttributes attributes = xmlReader.attributes();
                if (!attributes.hasAttribute(QLatin1String("p"))) {
                    result.errors.reserve(1);
                    result.errors.append("no 'p' attribute of r in file " + filePath);
                    return result;
                }
                valueP = attributes.value(QLatin1String("p")).toString();
            } else if (xmlReader.name() == "measCollec") {
                QXmlStreamAttributes attributes = xmlReader.attributes();
                if (attributes.hasAttribute(QLatin1String("beginTime"))) {
                    QString dateText = attributes.value(QLatin1String("beginTime")).toString();
                    QDateTime dateTime = QDateTime::fromString(dateText, Qt::ISODate);
                    if (dateTime.isValid()) {
                        result.dateTime = dateTime.toString(DT_FORMAT_IN_FILE);
                    } else {
                        result.errors.reserve(1);
                        result.errors.append("invalid date time format in file " + filePath);
                        return result;
                    }
                }
            }
        } else if (tokenType == QXmlStreamReader::EndElement) {
            if (xmlReader.name() == "measType") {
                isMeasTypeElement = false;
            } else if (xmlReader.name() == "r") {
                isRElement = false;
            }
        } else if (tokenType == QXmlStreamReader::Characters) {
            if (isMeasTypeElement) {
                pMeasType.insert(valueP, xmlReader.text().toString());
            } else if (isRElement) {
                const QString &measType = pMeasType[valueP];
                int index = indexes[objLdn + ',' + measType];
                values[index] = xmlReader.text().toString();
            }
        } else if (tokenType == QXmlStreamReader::Invalid) {
            result.errors.reserve(1);
            result.errors.append("failed to parse KCI/KPI file " + filePath + ": " + xmlReader.errorString());
            return result;
        }
    }

    result.values.swap(values);
    return result;
}

static void writeXmlData(GzipFile &fileWriter, XmlDataResult &finalResult, const XmlDataResult &intermediateResult)
{
    if (intermediateResult.errors.isEmpty() && !intermediateResult.values.isEmpty()) {
        fileWriter.write(intermediateResult.dateTime);
        fileWriter.write(";", 1);

        for (auto iter = intermediateResult.values.begin(); iter != intermediateResult.values.end() - 1; ++iter) {
            fileWriter.write(*iter);
            fileWriter.write(";", 1);
        }

        fileWriter.write(intermediateResult.values.back());
        fileWriter.write("\n", 1);
    } else {
        finalResult.errors.append(intermediateResult.errors);
    }
}

static QString getGwNameFromFileName(const QString &fileName)
{
    QVector<QStringRef> refs = fileName.splitRef('_');
    refs = refs.back().split('.');
    return refs.first().toString();
}

static QString getTimeFromFileName(const QString &fileName)
{
    return fileName.mid(1, 13);
}

static QString getOutputFilePath(const QStringList &filePaths)
{
    QFileInfo fileInfo(filePaths.first());
    QString fileName = fileInfo.fileName();
    QString outputFileName = getGwNameFromFileName(fileName);
    outputFileName += "__";
    outputFileName += getTimeFromFileName(fileName);

    if (filePaths.size() > 1) {
        fileInfo = QFileInfo(filePaths.last());
        fileName = fileInfo.fileName();
        outputFileName += "-";
        outputFileName += getTimeFromFileName(fileName);
    }

    outputFileName += ".csv.gz";

    return fileInfo.absoluteDir().absoluteFilePath(outputFileName);
}

static bool checkKciKpiFileNames(const QStringList &filePaths, QString &error)
{
    QRegExp fileNameExp(QStringLiteral("^A\\d{8}\\.\\d{4}[+-]\\d{4}-\\d{4}[+-]\\d{4}.+\\.xml(\\.gz)?$"));
    for (const QString &filePath : filePaths) {
        QString fileName = QFileInfo(filePath).fileName();
        if (!fileNameExp.exactMatch(fileName)) {
            error = "invalid KCI/KPI file name " + fileName;
            return false;
        }
    }
    return true;
}

QString StatisticsFileParser::kciKpiToCsvFormat(QStringList &filePaths, QString &error)
{
    if (!checkKciKpiFileNames(filePaths, error)) {
        return QString();
    }

    std::sort(filePaths.begin(), filePaths.end());

    volatile bool working = true;
    m_dialog.setLabelText(QStringLiteral("Parsing statistics names from the selected files..."));

    QFutureWatcher<XmlHeaderResult> watcher;
    QObject::connect(&m_dialog, &ProgressDialog::canceling, [&working, &watcher]() {
        working = false;
        watcher.cancel();
    });
    QObject::connect(&watcher, SIGNAL(progressRangeChanged(int, int)), &m_dialog, SLOT(setRange(int, int)));
    QObject::connect(&watcher, SIGNAL(progressValueChanged(int)), &m_dialog, SLOT(setValue(int)));
    QObject::connect(&watcher, SIGNAL(finished()), &m_dialog, SLOT(accept()));
    
    watcher.setFuture(QtConcurrent::mappedReduced(filePaths,
        std::bind(parseXmlHeader, std::ref(working), std::placeholders::_1),
        mergeXmlHeader));
    m_dialog.exec();

    if (watcher.isCanceled()) {
        return QString();
    }

    XmlHeaderResult headerResult = watcher.result();
    if (!headerResult.errors.isEmpty()) {
        error = headerResult.errors.first();
        return QString();
    }

    GzipFile fileWriter;
    QString destFilePath = getOutputFilePath(filePaths);
    if (!fileWriter.open(destFilePath, QIODevice::WriteOnly)) {
        error = "failed to open " + destFilePath;
        return QString();
    }

    QHash<QString, int> indexes;
    QFutureWatcher<void> writeXmlHeaderWatcher;
    QObject::connect(&m_dialog, &ProgressDialog::canceling, [&working]() {
        working = false;
    });
    QObject::connect(&writeXmlHeaderWatcher, SIGNAL(finished()), &m_dialog, SLOT(accept()));

    m_dialog.setValue(0);
    m_dialog.setRange(0, 100);
    m_dialog.setLabelText(QStringLiteral("Writing statistics names to file..."));

    writeXmlHeaderWatcher.setFuture(QtConcurrent::run(
        std::bind(writeXmlHeader, std::ref(m_dialog), std::cref(working), std::ref(fileWriter),
            std::cref(headerResult.root), std::ref(indexes))));
    m_dialog.exec();

    if (!working) {
        return QString();
    }

    QFutureWatcher<XmlDataResult> xmlDataWatcher;
    QObject::connect(&m_dialog, &ProgressDialog::canceling, [&working, &xmlDataWatcher]() {
        working = false;
        xmlDataWatcher.cancel();
    });
    QObject::connect(&xmlDataWatcher, SIGNAL(progressRangeChanged(int, int)), &m_dialog, SLOT(setRange(int, int)));
    QObject::connect(&xmlDataWatcher, SIGNAL(progressValueChanged(int)), &m_dialog, SLOT(setValue(int)));
    QObject::connect(&xmlDataWatcher, SIGNAL(finished()), &m_dialog, SLOT(accept()));

    m_dialog.setLabelText(QStringLiteral("Writing statistics data to file..."));

    xmlDataWatcher.setFuture(QtConcurrent::mappedReduced<XmlDataResult>(filePaths,
        std::bind(parseXmlData, std::placeholders::_1, std::cref(indexes), std::cref(working)),
        std::bind(writeXmlData, std::ref(fileWriter), std::placeholders::_1, std::placeholders::_2),
        QtConcurrent::OrderedReduce | QtConcurrent::SequentialReduce));
    m_dialog.exec();

    if (xmlDataWatcher.isCanceled()) {
        return QString();
    }

    XmlDataResult dataResult = xmlDataWatcher.result();
    if (!dataResult.errors.isEmpty()) {
        error = dataResult.errors.first();
        return QString();
    }

    return destFilePath;
}

QString StatisticsFileParser::getNodeFromFileName(const QString &fileName)
{
    int pos = fileName.indexOf(QStringLiteral("__"));
    Q_ASSERT(pos > 0);
    return fileName.left(pos);
}
