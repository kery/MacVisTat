#include "statisticsfileparser.h"
#include "progressdialog.h"
#include "gzipfile.h"
#include "utils.h"
#include <pcre.h>
#include <expat.h>
#include <QtConcurrent>
#include <functional>
#include <set>
#include <memory>
#include <unordered_map>

#define KPIKCI_A_FILE_PATTERN "^A\\d{8}\\.\\d{4}[+-]\\d{4}-\\d{4}[+-]\\d{4}(_-.+?)?(_.+?)?(_-_.+?)?\\.xml(\\.gz)?$"
#define KPIKCI_C_FILE_PATTERN "^C\\d{8}\\.\\d{4}[+-]\\d{4}-\\d{8}\\.\\d{4}[+-]\\d{4}(_-.+?)?(_.+?)?(_-_.+?)?\\.xml(\\.gz)?$"

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
    Statistics::NameDataMap ndm;
    QString error;
};

static FileDataResult doParseFileData(const StatisticsFileParser::IndexNameMap &inm,
                      ProgressDialog &dialog,
                      volatile bool &working,
                      const QString &path)
{
    FileDataResult result;
    std::string line;
    GzipFile reader;

    if (!reader.open(path)) {
        result.error = "failed to open ";
        result.error += path;
        return result;
    }

    // Read the header
    if (!reader.readLine(line)) {
        result.error = "failed to read header of ";
        result.error += path;
        return result;
    }

    QCPData data;
    int progress = 0;
    QList<int> indexes = inm.keys();

    const int BUFFER_SIZE = 4096;
    int index = 2;
    int parsed = 0;
    int copied = 0;
    std::unique_ptr<char[]> buffer(new char[BUFFER_SIZE]);
    const char *ptr = nullptr;

    int bytes = reader.read(buffer.get(), BUFFER_SIZE);
    if (bytes <= 0) {
        result.error = "failed to read data from ";
        result.error += path;
        return result;
    }

    int len = bytes;
    const char *newline = buffer.get();
    while (working) {
        int newProgress = reader.progress();
        if (newProgress > progress) {
            QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection,
                Q_ARG(int, newProgress));
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
                result.error = "invalid date time format in ";
                result.error += path;
                return result;
            }
        }
        char *suspectFlag;
        const char *semicolon;
        while ((semicolon = searchr(ptr, len, ';', &newline)) != nullptr) {
            if (index == indexes.at(parsed)) {
                if (*ptr != ';') {
                    data.value = strtod(ptr, &suspectFlag);
                    data.valueErrorMinus = *suspectFlag == 's' ? 1.0 : 0;
                    result.ndm[inm.value(index)].insert(data.key, data);
                }
                if (++parsed == indexes.size()) {
                    len -= semicolon - ptr;
                    ptr = semicolon;
                    while ((newline = (const char *)memchr(ptr, '\n', len)) == NULL) {
                        if ((bytes = reader.read(buffer.get(), BUFFER_SIZE)) <= 0) {
                            return result;
                        }
                        newProgress = reader.progress();
                        if (newProgress > progress) {
                            QMetaObject::invokeMethod(&dialog, "setValue", Qt::QueuedConnection,
                                Q_ARG(int, newProgress));
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
            if (parsed < indexes.size() && index == indexes.at(parsed) && *ptr != ';') {
                data.value = strtod(ptr, &suspectFlag);
                data.valueErrorMinus = *suspectFlag == 's' ? 1.0 : 0;
                result.ndm[inm.value(index)].insert(data.key, data);
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

bool StatisticsFileParser::parseFileData(const IndexNameMap &inm, const QString &path,
    Statistics::NameDataMap &ndm, QString &error)
{
    volatile bool working = true;
    auto parseRunner = std::bind(doParseFileData, std::cref(inm), std::ref(m_dialog), std::ref(working), std::cref(path));

    m_dialog.setRange(0, 100);
    m_dialog.setValue(0);
    QObject::connect(&m_dialog, &ProgressDialog::canceling, [&working] () {
        working = false;
    });
    QFutureWatcher<FileDataResult> watcher;
    QObject::connect(&watcher, SIGNAL(finished()), &m_dialog, SLOT(accept()));
    watcher.setFuture(QtConcurrent::run(parseRunner));
    m_dialog.exec();
    watcher.waitForFinished();

    if (working) {
        FileDataResult result = watcher.result();
        ndm.swap(result.ndm);
        error.swap(result.error);
        return true;
    }
    return false;
}

std::string parseHeader(const QString &path, QString &error)
{
    GzipFile reader;
    if (!reader.open(path)) {
        error = "failed to open ";
        error += path;
        return std::string();
    }

    std::string header;
    if (!reader.readLine(header)) {
        error = "failed to read from ";
        error += path;
        return std::string();
    }

    if (strncmp(header.c_str(), "##date;time;", 12) ||
        strcmp(header.c_str() + header.length() - 2, "##"))
    {
        error = "header format of ";
        error += path;
        error += "is invalid";
        return std::string();
    }
    return header;
}

std::string StatisticsFileParser::parseFileHeader(const QString &path, QString &error)
{
    auto parseRunner = std::bind(parseHeader, std::ref(path), std::ref(error));
    QFutureWatcher<std::string> watcher;
    QObject::connect(&watcher, SIGNAL(finished()), &m_dialog, SLOT(accept()));
    watcher.setFuture(QtConcurrent::run(parseRunner));
    m_dialog.exec();
    watcher.waitForFinished();
    return watcher.result();
}

struct kpiKciNameNode
{
    std::string name;
    mutable std::set<kpiKciNameNode> children;

    kpiKciNameNode();
    kpiKciNameNode(const char *s);
    kpiKciNameNode(const char *s, size_t n);
    kpiKciNameNode(const std::string &s);

    bool operator<(const kpiKciNameNode &other) const;
};

kpiKciNameNode::kpiKciNameNode()
{
}

kpiKciNameNode::kpiKciNameNode(const char *s) :
    name(s)
{
}

kpiKciNameNode::kpiKciNameNode(const char *s, size_t n) :
    name(s, n)
{
}

kpiKciNameNode::kpiKciNameNode(const std::string &s) :
    name(s)
{
}


bool kpiKciNameNode::operator<(const kpiKciNameNode &other) const
{
    return name < other.name;
}

static void mergekpiKciNameNode(const kpiKciNameNode &dest, const kpiKciNameNode &src)
{
    for (const kpiKciNameNode &child : src.children) {
        auto insertResult = dest.children.insert(child);
        mergekpiKciNameNode(*insertResult.first, child);
    }
}

static void feedFullkpiKciNames(const kpiKciNameNode &node, std::vector<std::string> &result, std::vector<const std::string *> &stack)
{
    if (node.children.empty()) {
        std::string fullName;
        for (const std::string *item : stack) {
            fullName += *item;
            fullName += ',';
        }
        fullName += node.name;
        result.push_back(std::move(fullName));
    } else {
        stack.push_back(&node.name);
        for (const kpiKciNameNode &child : node.children) {
            feedFullkpiKciNames(child, result, stack);
        }
        stack.pop_back();
    }
}

static std::vector<std::string> genFullkpiKciNames(const kpiKciNameNode &root)
{
    std::vector<std::string> result;
    for (const kpiKciNameNode &child : root.children) {
        std::vector<const std::string *> stack;
        feedFullkpiKciNames(child, result, stack);
    }
    return result;
}

struct XmlHeaderResult
{
    kpiKciNameNode root;
    QVector<QString> errors;
};

enum XmlElementName {
    XEN_notCare,
    XEN_measInfo,
    XEN_measType,
    XEN_r,
    XEN_suspect
};

struct XmlHeaderUserData
{
    XmlElementName elName;
    std::string measInfoId;
    std::string character;
    std::vector<std::string> measTypes;
    XML_Parser parser;
    const QString *filePath;
    XmlHeaderResult *result;
};

static const char *expatHelperGetAtt(const char *name, const char **atts)
{
    for (int i = 0; atts[i]; i += 2) {
        if (strcmp(atts[i], name) == 0) {
            return atts[i + 1];
        }
    }
    return nullptr;
}

static const char *getMeasInfoId(const char **atts) {
    const char *measInfoId = expatHelperGetAtt("measInfoId", atts);
    if (measInfoId != nullptr) {
        return measInfoId;
    }
    return "UnknownModule";
}

static void headerStartElementHandler(void *ud, const char *name, const char **atts)
{
    XmlHeaderUserData *userData = (XmlHeaderUserData *)ud;

    if (strcmp(name, "measType") == 0) {
        userData->elName = XEN_measType;
        return;
    }

    if (strcmp(name, "measInfo") == 0) {
        userData->measInfoId = getMeasInfoId(atts);
        return;
    }

    if (strcmp(name, "measValue")) {
        return;
    }

    const char *measObjLdn = expatHelperGetAtt("measObjLdn", atts);
    if (measObjLdn == nullptr) {
        userData->result->errors.reserve(1);
        userData->result->errors.append(QStringLiteral("measObjLdn is missing in file %1:%2")
                                        .arg(*userData->filePath)
                                        .arg(XML_GetCurrentLineNumber(userData->parser)));
        XML_StopParser(userData->parser, XML_FALSE);
        return;
    }

    const kpiKciNameNode *nameNode = &userData->result->root;
    auto insertResult = nameNode->children.insert(kpiKciNameNode(userData->measInfoId));
    nameNode = &(*insertResult.first);

    const char *comma;
    while ((comma = strchr(measObjLdn, ',')) != nullptr) {
        insertResult = nameNode->children.insert(kpiKciNameNode(measObjLdn, comma - measObjLdn));
        nameNode = &(*insertResult.first);
        measObjLdn = comma + 1;
    }

    insertResult = nameNode->children.insert(kpiKciNameNode(measObjLdn));
    nameNode = &(*insertResult.first);

    for (const std::string &measType : userData->measTypes) {
        nameNode->children.insert(kpiKciNameNode(measType));
    }
}

static void headerEndElementHandler(void *ud, const char *name)
{
    XmlHeaderUserData *userData = (XmlHeaderUserData *)ud;

    if (strcmp(name, "measType") == 0) {
        userData->elName = XEN_notCare;

        userData->measTypes.push_back(userData->character);
        userData->character.clear();
    } else if (strcmp(name, "measInfo") == 0) {
        userData->measTypes.clear();
    }
}

static void headerCharacterHandler(void *ud, const char *s, int len)
{
    XmlHeaderUserData *userData = (XmlHeaderUserData *)ud;

    if (userData->elName == XEN_measType) {
        userData->character.append(s, len);
    }
}

static XmlHeaderResult parseXmlHeader(volatile const bool &working, const QString &filePath)
{
    XmlHeaderResult result;

    GzipFile fileReader;
    if (!fileReader.open(filePath)) {
        result.errors.reserve(1);
        result.errors.append("failed to open " + filePath);
        return result;
    }

    XML_Parser parser = XML_ParserCreate(NULL);

    XmlHeaderUserData userData;
    userData.elName = XEN_notCare;
    userData.parser = parser;
    userData.filePath = &filePath;
    userData.result = &result;

    XML_SetUserData(parser, &userData);
    XML_SetElementHandler(parser, headerStartElementHandler, headerEndElementHandler);
    XML_SetCharacterDataHandler(parser, headerCharacterHandler);

    int len;
    char buf[BUFSIZ];
    while (working && result.errors.isEmpty() && (len = fileReader.read(buf, sizeof(buf))) > 0) {
        int isFinal = len < (int)sizeof(buf);
        if (XML_Parse(parser, buf, len, isFinal) == XML_STATUS_ERROR) {
            result.errors.reserve(1);
            result.errors.append(QStringLiteral("failed to parse KPI-KCI file %1: %2, line %3").arg(filePath)
                                 .arg(XML_ErrorString(XML_GetErrorCode(parser)))
                                 .arg(XML_GetCurrentLineNumber(parser)));
            break;
        }
    }

    XML_ParserFree(parser);

    return result;
}

static void mergeXmlHeader(XmlHeaderResult &finalResult, const XmlHeaderResult &intermediateResult)
{
    if (intermediateResult.errors.isEmpty()) {
        mergekpiKciNameNode(finalResult.root, intermediateResult.root);
    } else {
        finalResult.errors.append(intermediateResult.errors);
    }
}

static void writeXmlHeader(ProgressDialog &dialog, volatile const bool &working,
    GzipFile &fileWriter, const kpiKciNameNode &root, std::unordered_map<std::string, int> &indexes)
{
    int progress = 0;
    std::vector<std::string> fullNames = genFullkpiKciNames(root);

    fileWriter.write("##date;time", 11);

    for (int index = 0; working && index < (int)fullNames.size(); ++index) {
        const std::string &fullName = fullNames[index];
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

struct MeasData {
    std::string dateTime;
    std::vector<std::string> values;

    MeasData() {}
    MeasData(size_t size) : values(size) {}
};

struct XmlDataResult
{
    std::vector<MeasData> datas;
    QVector<QString> errors;
};

struct XmlDataUserData
{
    bool suspectFlag;
    XmlElementName elName;
    QDateTime tempEndTime;
    std::string measInfoId;
    std::string valueP;
    std::string objLdn;
    std::string character;
    std::vector<int> tempIndexes;
    std::unordered_map<std::string, std::string> pMeasType;
    const std::unordered_map<std::string, int> *indexes;
    XML_Parser parser;
    const QString *filePath;
    XmlDataResult *result;
};

static void dataStartElementHandler(void *ud, const char *name, const char **atts)
{
    XmlDataUserData *userData = (XmlDataUserData *)ud;

    if (strcmp(name, "measType") == 0) {
        userData->elName = XEN_measType;

        const char *attP = expatHelperGetAtt("p", atts);
        if (attP == nullptr) {
            userData->result->errors.reserve(1);
            userData->result->errors.append(QStringLiteral("no 'p' attribute in file %1:%2")
                                            .arg(*userData->filePath)
                                            .arg(XML_GetCurrentLineNumber(userData->parser)));

            XML_StopParser(userData->parser, XML_FALSE);
            return;
        }

        userData->valueP = attP;
    } else if (strcmp(name, "measValue") == 0) {
        userData->objLdn = expatHelperGetAtt("measObjLdn", atts);
    } else if (strcmp(name, "r") == 0) {
        userData->elName = XEN_r;

        const char *attP = expatHelperGetAtt("p", atts);
        if (attP == nullptr) {
            userData->result->errors.reserve(1);
            userData->result->errors.append(QStringLiteral("no 'p' attribute in file %1:%2")
                                            .arg(*userData->filePath)
                                            .arg(XML_GetCurrentLineNumber(userData->parser)));

            XML_StopParser(userData->parser, XML_FALSE);
            return;
        }

        userData->valueP = attP;
    } else if (strcmp(name, "suspect") == 0) {
        userData->elName = XEN_suspect;
    } else if (strcmp(name, "measInfo") == 0) {
        userData->measInfoId = getMeasInfoId(atts);
    } else if (strcmp(name, "granPeriod") == 0) {
        const char *endTime = expatHelperGetAtt("endTime", atts);
        QDateTime dateTime = QDateTime::fromString(endTime, Qt::ISODate);
        if (dateTime.isNull()) {
            userData->result->errors.reserve(1);
            userData->result->errors.append(QStringLiteral("invalid date time format in file %1:%2")
                                            .arg(*userData->filePath)
                                            .arg(XML_GetCurrentLineNumber(userData->parser)));
            XML_StopParser(userData->parser, XML_FALSE);
            return;
        }

        if (dateTime != userData->tempEndTime) {
            userData->tempEndTime = dateTime;
            userData->result->datas.push_back(MeasData(userData->indexes->size()));
            userData->result->datas.back().dateTime = dateTime.toString(DT_FORMAT_IN_FILE).toStdString();
        }
    }
}

static void dataEndElementHandler(void *ud, const char *name)
{
    XmlDataUserData *userData = (XmlDataUserData *)ud;

    if (strcmp(name, "measType") == 0) {
        userData->elName = XEN_notCare;
        userData->pMeasType[userData->valueP].swap(userData->character);
    } else if (strcmp(name, "r") == 0) {
        userData->elName = XEN_notCare;

        const std::string &measType = userData->pMeasType[userData->valueP];
        const std::string counterName = userData->measInfoId + ',' + userData->objLdn + ',' + measType;
        int index = userData->indexes->at(counterName);
        userData->tempIndexes.push_back(index);
        userData->result->datas.back().values[index].swap(userData->character);
    } else if (strcmp(name, "measValue") == 0) {
        if (userData->suspectFlag) {
            userData->suspectFlag = false;
            for (const int &index : userData->tempIndexes) {
                userData->result->datas.back().values[index] += 's';
            }
        }

        userData->tempIndexes.clear();
    } else if (strcmp(name, "suspect") == 0) {
        userData->elName = XEN_notCare;

        if (userData->character == "true") {
            userData->suspectFlag = true;
        }

        userData->character.clear();
    } else if (strcmp(name, "measInfo") == 0) {
        userData->pMeasType.clear();
    }
}

static void dataCharacterHandler(void *ud, const char *s, int len)
{
    XmlDataUserData *userData = (XmlDataUserData *)ud;

    switch (userData->elName) {
    case XEN_measType:
    case XEN_r:
    case XEN_suspect:
        userData->character.append(s, len);
        break;
    default:
        break;
    }
}

static XmlDataResult parseXmlData(const QString &filePath, const std::unordered_map<std::string, int> &indexes,
    volatile const bool &working)
{
    XmlDataResult result;

    GzipFile fileReader;
    if (!fileReader.open(filePath)) {
        result.errors.reserve(1);
        result.errors.append("failed to open " + filePath);
        return result;
    }

    XML_Parser parser = XML_ParserCreate(NULL);

    XmlDataUserData userData;
    userData.suspectFlag = false;
    userData.elName = XEN_notCare;
    userData.indexes = &indexes;
    userData.parser = parser;
    userData.filePath = &filePath;
    userData.result = &result;

    XML_SetUserData(parser, &userData);
    XML_SetElementHandler(parser, dataStartElementHandler, dataEndElementHandler);
    XML_SetCharacterDataHandler(parser, dataCharacterHandler);

    int len;
    char buf[BUFSIZ];
    while (working && result.errors.isEmpty() && (len = fileReader.read(buf, sizeof(buf))) > 0) {
        int isFinal = len < (int)sizeof(buf);
        if (XML_Parse(parser, buf, len, isFinal) == XML_STATUS_ERROR) {
            result.errors.reserve(1);
            result.errors.append(QStringLiteral("failed to parse KPI-KCI file %1: %2, line %3").arg(filePath)
                                 .arg(XML_ErrorString(XML_GetErrorCode(parser)))
                                 .arg(XML_GetCurrentLineNumber(parser)));
            break;
        }
    }

    XML_ParserFree(parser);

    return result;
}

static void writeXmlData(GzipFile &fileWriter, XmlDataResult &finalResult, const XmlDataResult &intermediateResult)
{
    if (intermediateResult.errors.isEmpty()) {
        for (const MeasData &data : intermediateResult.datas) {
            fileWriter.write(data.dateTime);
            fileWriter.write(";", 1);

            for (auto iter = data.values.begin(); iter != data.values.end() - 1; ++iter) {
                fileWriter.write(*iter);
                fileWriter.write(";", 1);
            }

            fileWriter.write(data.values.back());
            fileWriter.write("\n", 1);
        }
    } else {
        finalResult.errors.append(intermediateResult.errors);
    }
}

static QString getGwNameFromFileName(const QString &fileName)
{
    const char *err;
    int errOffset;
    pcre *re;
    QString gwName("MG");

    if (fileName.startsWith('A', Qt::CaseInsensitive)) {
        re = pcre_compile(KPIKCI_A_FILE_PATTERN, PCRE_CASELESS, &err, &errOffset, NULL);
    } else {
        re = pcre_compile(KPIKCI_C_FILE_PATTERN, PCRE_CASELESS, &err, &errOffset, NULL);
    }

    if (re) {
        const int OVECCOUNT = 30;
        int ovector[OVECCOUNT];
        std::string fname = fileName.toStdString();
        int ret = pcre_exec(re, NULL, fname.c_str(), (int)fname.length(), 0, 0, ovector, OVECCOUNT);
        if (ret != PCRE_ERROR_NOMATCH) {
            const char *subStr;
            if (pcre_get_substring(fname.c_str(), ovector, ret, 2, &subStr) > 0) {
                gwName = subStr + 1; // exclude the leading '_'
                pcre_free_substring(subStr);
            }
        }
        pcre_free(re);
    }

    return gwName;
}

static QString getFirstEndTimeFromFile(const QString &path)
{
    GzipFile fileReader;
    if (fileReader.open(path)) {
        QXmlStreamReader xmlReader(&fileReader);

        while (!xmlReader.atEnd()) {
            QXmlStreamReader::TokenType tokenType = xmlReader.readNext();
            if (tokenType == QXmlStreamReader::StartElement && xmlReader.name() == "granPeriod") {
                QXmlStreamAttributes attributes = xmlReader.attributes();
                QString endTimeText = attributes.value(QLatin1String("endTime")).toString();
                QDateTime endTime = QDateTime::fromString(endTimeText, Qt::ISODate);
                return endTime.toString(DT_FORMAT_IN_FILE_NAME);
            }
        }
    }
    return QString();
}

static QString getEndTimeFromFileName(const QString &fileName)
{
    if (fileName.startsWith('A')) {
        QString endTime = fileName.mid(1, 8);
        endTime.append(".");

        QStringRef timeRef = fileName.midRef(20, 4);
        endTime.append(timeRef);
        return endTime;
    } else if(fileName.startsWith('C')) {
        return fileName.mid(20, 13);
    } else {
        return QString();
    }
}

static QString getOutputFilePath(const QStringList &filePaths)
{
    QFileInfo fileInfo(filePaths.first());
    QString outputFileName = getGwNameFromFileName(fileInfo.fileName());
    outputFileName += "__";
    outputFileName += getFirstEndTimeFromFile(filePaths.first());

    if (filePaths.size() > 1) {
        outputFileName += "-";
        outputFileName += getEndTimeFromFileName(QFileInfo(filePaths.last()).fileName());
    }

    outputFileName += ".csv.gz";

    return fileInfo.absoluteDir().absoluteFilePath(outputFileName);
}

static bool checkkpiKciFileNames(const QStringList &filePaths, QString &error)
{
    const char *err;
    int errOffset;
    pcre *re;

    if (QFileInfo(filePaths.first()).fileName().startsWith('A', Qt::CaseInsensitive)) {
        re = pcre_compile(KPIKCI_A_FILE_PATTERN, PCRE_CASELESS, &err, &errOffset, NULL);
    } else {
        re = pcre_compile(KPIKCI_C_FILE_PATTERN, PCRE_CASELESS, &err, &errOffset, NULL);
    }

    if (!re) {
        error = QStringLiteral("failed to compile regular expression: %1, offset %2").arg(err).arg(errOffset);
        return false;
    }

    const int OVECCOUNT = 30;
    int ovector[OVECCOUNT];

    for (const QString &filePath : filePaths) {
        std::string fileName = QFileInfo(filePath).fileName().toStdString();
        int ret = pcre_exec(re, NULL, fileName.c_str(), (int)fileName.length(), 0, 0, ovector, OVECCOUNT);
        if (ret == PCRE_ERROR_NOMATCH) {
            error = "invalid KPI-KCI file name " + QFileInfo(filePath).fileName();
            pcre_free(re);
            return false;
        }
    }

    pcre_free(re);

    return true;
}

QString StatisticsFileParser::kpiKciToCsvFormat(QStringList &filePaths, QString &error)
{
    if (!checkkpiKciFileNames(filePaths, error)) {
        return QString();
    }

    std::sort(filePaths.begin(), filePaths.end());

    if (filePaths.size() > 1 &&
            QFileInfo(filePaths.first()).fileName().at(0) != QFileInfo(filePaths.last()).fileName().at(0))
    {
        error = "multiple KPI-KCI file types are not allowed in conversion";
        return QString();
    }

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

    std::unordered_map<std::string, int> indexes;
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
