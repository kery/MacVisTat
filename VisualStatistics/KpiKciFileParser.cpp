#include "KpiKciFileParser.h"
#include "ProgressDialog.h"
#include "CounterName.h"
#include "GzipFile.h"
#include "OptionsDialog.h"
#include "GlobalDefines.h"
#include <QtConcurrent>

KpiKciFileParser::HeaderResult::HeaderResult()
{
    errors.reserve(1);
    paths.reserve(1);
}

KpiKciFileParser::MeasData::MeasData(size_t size) :
    values(size)
{
}

KpiKciFileParser::DataResult::DataResult()
{
    errors.reserve(1);
}

QRegularExpression KpiKciFileParser::mRegExpTypeA("^A(\\d{8}\\.\\d{4})([+-]\\d{4})-(\\d{4})[+-]\\d{4}(_-.+?)?(_.+?)?(_-_\\d+?)?(\\.xml(\\.gz)?)?$");
QRegularExpression KpiKciFileParser::mRegExpTypeC("^C(\\d{8}\\.\\d{4})([+-]\\d{4})-(\\d{8}\\.\\d{4})[+-]\\d{4}(_-.+?)?(_.+?)?(_-_\\d+?)?(\\.xml(\\.gz)?)?$");

KpiKciFileParser::KpiKciFileParser(QWidget *parent) :
    mParent(parent)
{
}

QString KpiKciFileParser::convertToCsv(QVector<QString> &paths, QVector<QString> &errors)
{
    QSettings setting;
    bool abortConvOnFailure = setting.value(OptionsDialog::sKeyAbortConvOnFailure, OptionsDialog::sDefAbortConvOnFailure).toBool();

    ProgressDialog dlg(mParent);
    dlg.setDescription(QStringLiteral("Sorting KPI/KCI files..."));
    dlg.setUndeterminable();
    dlg.setCancelButtonVisible(false);

    QVector<QString> sortErrors;
    QFutureWatcher<void> sortWatcher;
    QObject::connect(&sortWatcher, &QFutureWatcher<void>::finished, &dlg, &ProgressDialog::accept);
    sortWatcher.setFuture(QtConcurrent::run(std::bind(sortFiles, std::ref(paths), std::ref(sortErrors))));
    dlg.exec();
    sortWatcher.waitForFinished();

    sortErrors.swap(errors);
    if ((abortConvOnFailure && !errors.isEmpty()) || paths.isEmpty()) { return QString(); }

    volatile bool working = true;
    dlg.setDescription(QStringLiteral("Parsing counter names from KPI/KCI files..."));
    dlg.setCancelButtonVisible(true);

    QFutureWatcher<HeaderResult> hdrWatcher;
    QObject::connect(&dlg, &ProgressDialog::canceling, [&working, &hdrWatcher]() {
        working = false;
        hdrWatcher.cancel();
    });
    QObject::connect(&hdrWatcher, &QFutureWatcher<HeaderResult>::progressRangeChanged, &dlg, &ProgressDialog::setRange);
    QObject::connect(&hdrWatcher, &QFutureWatcher<HeaderResult>::progressValueChanged, &dlg, &ProgressDialog::setValue);
    QObject::connect(&hdrWatcher, &QFutureWatcher<HeaderResult>::finished, &dlg, &ProgressDialog::accept);

    hdrWatcher.setFuture(QtConcurrent::mappedReduced(paths, std::bind(parseHeader, std::placeholders::_1, std::ref(working)),
                                                     mergeHeaderResult));
    dlg.exec();
    hdrWatcher.waitForFinished();

    if (hdrWatcher.isCanceled()) { return QString(); }

    HeaderResult hdrResult = hdrWatcher.result();
    errors.append(hdrResult.errors);
    if (abortConvOnFailure && !hdrResult.errors.isEmpty()) { return QString(); }
    if (hdrResult.infoIdMap.empty()) {
        errors.append(QString("there is no counter in selected KPI/KCI files"));
        return QString();
    }
    for (const QString &failedPath : qAsConst(hdrResult.paths)) {
        paths.removeOne(failedPath);
    }
    if (paths.isEmpty()) { return QString(); }

    GzipFile writer;
    QString outPath = getOutputPath(paths);
    if (!writer.open(outPath, GzipFile::WriteOnly)) {
        errors.append("failed to open ");
        errors.last() += QDir::toNativeSeparators(outPath);
        return QString();
    }

    dlg.setRange(0, 100);
    dlg.setDescription(QStringLiteral("Writing counter names to CSV file..."));
    std::string offsetFromUtc = getOffsetFromUtc(paths.first());

    IndexMap indexMap;
    QFutureWatcher<void> writeHdrWatcher;
    QObject::connect(&dlg, &ProgressDialog::canceling, [&working]() { working = false; });
    QObject::connect(&writeHdrWatcher, &QFutureWatcher<void>::finished, &dlg, &ProgressDialog::accept);

    writeHdrWatcher.setFuture(QtConcurrent::run(std::bind(writeHeader,
                                                          std::cref(hdrResult.infoIdMap),
                                                          std::cref(offsetFromUtc),
                                                          std::cref(working),
                                                          std::ref(dlg),
                                                          std::ref(writer),
                                                          std::ref(indexMap))));
    dlg.exec();
    writeHdrWatcher.waitForFinished();

    if (!working) { return QString(); }

    QFutureWatcher<DataResult> dataWatcher;
    QObject::connect(&dlg, &ProgressDialog::canceling, [&working, &dataWatcher]() {
        working = false;
        dataWatcher.cancel();
    });
    QObject::connect(&dataWatcher, &QFutureWatcher<DataResult>::progressRangeChanged, &dlg, &ProgressDialog::setRange);
    QObject::connect(&dataWatcher, &QFutureWatcher<DataResult>::progressValueChanged, &dlg, &ProgressDialog::setValue);
    QObject::connect(&dataWatcher, &QFutureWatcher<DataResult>::finished, &dlg, &ProgressDialog::accept);
    dlg.setDescription(QStringLiteral("Writing counter values to CSV file..."));

    dataWatcher.setFuture(QtConcurrent::mappedReduced<DataResult>(paths,
        std::bind(parseData, std::placeholders::_1, std::cref(indexMap), std::cref(working)),
        std::bind(writeData, std::placeholders::_1, std::placeholders::_2, std::ref(writer)),
        QtConcurrent::OrderedReduce | QtConcurrent::SequentialReduce));
    dlg.exec();
    dataWatcher.waitForFinished();

    if (dataWatcher.isCanceled()) { return QString(); }

    DataResult dataResult = dataWatcher.result();
    errors.append(dataResult.errors);
    if (abortConvOnFailure && !dataResult.errors.isEmpty()) {
        writer.close();
        QFile::remove(outPath);
        return QString();
    }
    return outPath;
}

const char * KpiKciFileParser::findAttribute(const char *name, const char **atts)
{
    for (int i = 0; atts[i]; i += 2) {
        if (strcmp(atts[i], name) == 0) {
            return atts[i + 1];
        }
    }
    return nullptr;
}

QString KpiKciFileParser::getAttribute(const QString &path, XML_StartElementHandler handler)
{
    GzipFile reader;
    if (!reader.open(path, GzipFile::ReadOnly)) { return QString(); }

    QString result;
    XML_Parser parser = XML_ParserCreate(nullptr);
    XML_SetUserData(parser, &result);
    XML_SetStartElementHandler(parser, handler);

    int len;
    char buf[BUFSIZ];
    while (result.isNull() && (len = reader.read(buf, BUFSIZ)) > 0) {
        int isFinal = len < BUFSIZ;
        if (XML_Parse(parser, buf, len, isFinal) == XML_STATUS_ERROR) {
            break;
        }
    }
    XML_ParserFree(parser);
    return result;
}

const char * KpiKciFileParser::getMeasInfoId(const char **atts)
{
    const char *measInfoId = findAttribute("measInfoId", atts);
    if (measInfoId != nullptr) {
        return measInfoId;
    }
    return "";
}

std::string KpiKciFileParser::getOffsetFromUtc(const QString &path)
{
    QDateTime beginTime = QDateTime::fromString(getBeginTime(path), Qt::ISODate);
    if (beginTime.isValid()) {
        return std::to_string(beginTime.offsetFromUtc());
    }
    return std::string();
}

QString KpiKciFileParser::toIsoDateFormat(QString &&dateTime, const QStringRef &offsetFromUtc)
{
    QString result(std::move(dateTime));
    result.insert(4, '-').insert(7, '-').replace(10, 1, 'T').insert(13, ':');
    result.append(offsetFromUtc.left(3));
    result.append(':');
    result.append(offsetFromUtc.right(2));
    return result;
}

void KpiKciFileParser::getBeginTime_handler(void *ud, const char *name, const char **atts)
{
    if (strcmp(name, "measCollec")) { return; }

    const char *beginTime = findAttribute("beginTime", atts);
    if (beginTime != nullptr) {
        static_cast<QString *>(ud)->operator=(beginTime);
    }
}

QString KpiKciFileParser::getBeginTime(const QString &path)
{
    QString fileName = QFileInfo(path).fileName();
    if (fileName.startsWith('A')) {
        QRegularExpressionMatch match = mRegExpTypeA.match(fileName);
        if (match.hasMatch()) {
            QString beginTime = match.captured(1);
            QStringRef offsetFromUtc = match.capturedRef(2);
            return toIsoDateFormat(std::move(beginTime), offsetFromUtc);
        }
    } else if (fileName.startsWith('C')) {
        QRegularExpressionMatch match = mRegExpTypeC.match(fileName);
        if (match.hasMatch()) {
            QString beginTime = match.captured(1);
            QStringRef offsetFromUtc = match.capturedRef(2);
            return toIsoDateFormat(std::move(beginTime), offsetFromUtc);
        }
    }
    return getAttribute(path, getBeginTime_handler);
}

void KpiKciFileParser::getEndTime_handler(void *ud, const char *name, const char **atts)
{
    if (strcmp(name, "measCollec")) { return; }

    const char *endTime = findAttribute("endTime", atts);
    if (endTime != nullptr) {
        static_cast<QString *>(ud)->operator=(endTime);
    }
}

QString KpiKciFileParser::getEndTime(const QString &path)
{
    QString fileName = QFileInfo(path).fileName();
    // Date part of end time is not present in file name when type is "A" and
    // the file may cross two days (e.g. 23:45:00 ~ 00:00:00), in such case the
    // date is the day after the date in begin time. To simplify, get the end time
    // from file content in case type is "A".
    if (fileName.startsWith('C')) {
        QRegularExpressionMatch match = mRegExpTypeC.match(fileName);
        if (match.hasMatch()) {
            QString endTime = match.captured(3);
            QStringRef offsetFromUtc = match.capturedRef(2);
            return toIsoDateFormat(std::move(endTime), offsetFromUtc);
        }
    }
    return getAttribute(path, getEndTime_handler);
}

void KpiKciFileParser::sortFiles(QVector<QString> &paths, QVector<QString> &errors)
{
    QVector<QDateTime> beginTimeVector(paths.size());
    for (int i = 0; i < paths.size(); ++i) {
        beginTimeVector[i] = QDateTime::fromString(getBeginTime(paths[i]), Qt::ISODate);
        if (!beginTimeVector[i].isValid()) {
            errors.append("failed to get begin time from ");
            errors.last() += QDir::toNativeSeparators(paths[i]);
        }
    }

    QVector<QPair<QDateTime, QString>> pairs;
    pairs.reserve(paths.size());
    for (int i = 0; i < paths.size(); ++i) {
        if (beginTimeVector[i].isValid()) {
            pairs.append(QPair<QDateTime, QString>());
            pairs.last().first.swap(beginTimeVector[i]);
            pairs.last().second.swap(paths[i]);
        }
    }

    std::sort(pairs.begin(), pairs.end());
    paths.resize(pairs.size());
    for (int i = 0; i < pairs.size(); ++i) {
        paths[i].swap(pairs[i].second);
    }
}

void KpiKciFileParser::getManagedElement_handler(void *ud, const char *name, const char **atts)
{
    if (strcmp(name, "fileSender")) { return; }

    const char *localDn = findAttribute("localDn", atts);
    if (localDn != nullptr) {
        const char *me = "ManagedElement=";
        const char *pos = strstr(localDn, me);
        if (pos != nullptr) {
            static_cast<QString *>(ud)->operator=(pos + strlen(me));
            return;
        }
    }

    // stop parsing
    static_cast<QString *>(ud)->operator=("");
}

QString KpiKciFileParser::getManagedElement(const QString &path)
{
    return getAttribute(path, getManagedElement_handler);
}

QString KpiKciFileParser::getUniqueIdFromFileName(const QString &path)
{
    QStringRef uniqueId;
    // Must valid during uniqueId's lifetime so we declare it here rather than the if block.
    QRegularExpressionMatch match;
    QString fileName = QFileInfo(path).fileName();
    if (fileName.startsWith('A')) {
        match = mRegExpTypeA.match(fileName);
        if (match.hasMatch()) {
            uniqueId = match.capturedRef(5);
        }
    } else if (fileName.startsWith('C')) {
        match = mRegExpTypeC.match(fileName);
        if (match.hasMatch()) {
            uniqueId = match.capturedRef(5);
        }
    }
    return uniqueId.isEmpty() ? QString() : uniqueId.mid(1).toString();
}

void KpiKciFileParser::getFirstGranPeriodEndTime_handler(void *ud, const char *name, const char **atts)
{
    if (strcmp(name, "granPeriod")) { return; }

    const char *endTime = findAttribute("endTime", atts);
    if (endTime != nullptr) {
        static_cast<QString *>(ud)->operator=(endTime);
    }
}

QString KpiKciFileParser::getFirstGranPeriodEndTime(const QString &path)
{
    return getAttribute(path, getFirstGranPeriodEndTime_handler);
}

QString KpiKciFileParser::getOutputPath(const QVector<QString> &paths)
{
    QString nodeName = getManagedElement(paths.first());
    if (nodeName.isEmpty()) {
        nodeName = getUniqueIdFromFileName(paths.first());
    }
    QString firstEndTime = getFirstGranPeriodEndTime(paths.first());
    QString endTime = getEndTime(paths.last());
    QDateTime dtFirstEndTime = QDateTime::fromString(firstEndTime, Qt::ISODate);
    QDateTime dtEndTime = QDateTime::fromString(endTime, Qt::ISODate);

    QString fileName = QStringLiteral("%1_%2-%3.csv.gz").arg(nodeName,
                                                             dtFirstEndTime.toString(DTFMT_IN_FILENAME),
                                                             dtEndTime.toString(DTFMT_IN_FILENAME));
    QFileInfo fileInfo(paths.first());
    return fileInfo.absoluteDir().absoluteFilePath(fileName);
}

void KpiKciFileParser::startElement_headerHandler(void *ud, const char *name, const char **atts)
{
    HeaderUserData *userData = static_cast<HeaderUserData *>(ud);

    if (strcmp(name, "measType") == 0) {
        userData->elemName = xenMeasType;
    } else if (strcmp(name, "measValue") == 0) {
        const char *measObjLdn = findAttribute("measObjLdn", atts);
        if (measObjLdn != nullptr) {
            ObjLdnMap &objMap = userData->result->infoIdMap[userData->measInfoId];
            std::set<std::string> &measTypes = objMap[measObjLdn];
            measTypes.insert(userData->measTypes.begin(), userData->measTypes.end());
        } else {
            userData->result->errors.append(QStringLiteral("measObjLdn is missing in %1:%2")
                                            .arg(QDir::toNativeSeparators(*userData->path))
                                            .arg(XML_GetCurrentLineNumber(userData->parser)));
            XML_StopParser(userData->parser, XML_FALSE);
        }
    } else if (strcmp(name, "measInfo") == 0) {
        userData->measInfoId = getMeasInfoId(atts);
    }
}

void KpiKciFileParser::endElement_headerHandler(void *ud, const char *name)
{
    HeaderUserData *userData = static_cast<HeaderUserData *>(ud);

    if (strcmp(name, "measType") == 0) {
        userData->elemName = xenNotCare;
        userData->measTypes.push_back(std::move(userData->character));
    } else if (strcmp(name, "measInfo") == 0) {
        userData->measTypes.clear();
    }
}

void KpiKciFileParser::character_headerHandler(void *ud, const char *s, int len)
{
    HeaderUserData *userData = static_cast<HeaderUserData *>(ud);

    if (userData->elemName == xenMeasType) {
        userData->character.append(s, len);
    }
}

void KpiKciFileParser::mergeHeaderResult(HeaderResult &finalResult, const HeaderResult &intermResult)
{
    if (intermResult.errors.isEmpty()) {
        for (auto iter = intermResult.infoIdMap.begin(); iter != intermResult.infoIdMap.end(); ++iter) {
            ObjLdnMap &finalObjMap = finalResult.infoIdMap[iter->first];
            for (auto iterInner = iter->second.begin(); iterInner != iter->second.end(); ++iterInner) {
                std::set<std::string> &measTypes = finalObjMap[iterInner->first];
                if (measTypes != iterInner->second) {
                    measTypes.insert(iterInner->second.begin(), iterInner->second.end());
                }
            }
        }
    } else {
        finalResult.errors.append(intermResult.errors);
        finalResult.paths.append(intermResult.paths);
    }
}

KpiKciFileParser::HeaderResult KpiKciFileParser::parseHeader(const QString &path, volatile const bool &working)
{
    HeaderResult result;
    result.paths.append(path);

    GzipFile reader;
    if (!reader.open(path, GzipFile::ReadOnly)) {
        result.errors.append("failed to open ");
        result.errors.last() += QDir::toNativeSeparators(path);
        return result;
    }

    XML_Parser parser = XML_ParserCreate(nullptr);

    HeaderUserData userData;
    userData.elemName = xenNotCare;
    userData.parser = parser;
    userData.path = &path;
    userData.result = &result;

    XML_SetUserData(parser, &userData);
    XML_SetElementHandler(parser, startElement_headerHandler, endElement_headerHandler);
    XML_SetCharacterDataHandler(parser, character_headerHandler);

    int len;
    char buf[BUFSIZ];
    while (working && result.errors.isEmpty() && (len = reader.read(buf, BUFSIZ)) > 0) {
        int isFinal = len < BUFSIZ;
        if (XML_Parse(parser, buf, len, isFinal) == XML_STATUS_ERROR) {
            result.errors.append(QStringLiteral("failed to parse header in %1:%2: %3")
                                 .arg(QDir::toNativeSeparators(path))
                                 .arg(XML_GetCurrentLineNumber(parser))
                                 .arg(XML_ErrorString(XML_GetErrorCode(parser))));
            break;
        }
    }
    XML_ParserFree(parser);
    return result;
}

void KpiKciFileParser::writeHeader(const InfoIdMap &iim, const std::string &offsetFromUtc, volatile const bool &working,
                                   ProgressDialog &dlg, GzipFile &writer, IndexMap &indexMap)
{
    int index = 0, infoIdIndex = 0, progress = 0;

    writer.write("##date;time", 11);
    writer.write(offsetFromUtc);

    std::string counterName;
    for (auto iter = iim.begin(); working && iter != iim.end(); ++iter) {
        const std::string &measInfoId = iter->first;
        counterName.append(measInfoId);
        for (auto iterInner = iter->second.begin(); iterInner != iter->second.end(); ++iterInner) {
            const std::string &measObjLdn = iterInner->first;
            counterName.push_back(CounterName::sModuleSeparator.toLatin1());
            counterName.append(measObjLdn);
            counterName.push_back(CounterName::sIndexesSeparator.toLatin1());
            for (const std::string &measType : iterInner->second) {
                counterName.append(measType);
                indexMap[counterName] = index++;
                writer.write(";", 1);
                writer.write(counterName);
                counterName.erase(counterName.end() - measType.length(), counterName.end());
            }
            counterName.erase(counterName.end() - measObjLdn.length() - 2, counterName.end());
        }
        counterName.erase(counterName.end() - measInfoId.length(), counterName.end());

        int newProgress = ++infoIdIndex / static_cast<int>(iim.size()) * 100;
        if (newProgress > progress) {
            progress = newProgress;
            QMetaObject::invokeMethod(&dlg, "setValue", Qt::QueuedConnection, Q_ARG(int, progress));
        }
    }

    writer.write("##\n", 3);
}

void KpiKciFileParser::startElement_dataHandler(void *ud, const char *name, const char **atts)
{
    DataUserData *userData = static_cast<DataUserData *>(ud);

    if (strcmp(name, "measType") == 0) {
        userData->elemName = xenMeasType;

        const char *attP = findAttribute("p", atts);
        if (attP != nullptr) {
            userData->attP = attP;
        } else {
            userData->result->errors.append(QStringLiteral("no 'p' attribute in %1:%2")
                                            .arg(QDir::toNativeSeparators(*userData->path))
                                            .arg(XML_GetCurrentLineNumber(userData->parser)));
            XML_StopParser(userData->parser, XML_FALSE);
        }
    } else if (strcmp(name, "r") == 0) {
        userData->elemName = xenR;

        const char *attP = findAttribute("p", atts);
        if (attP != nullptr) {
            userData->attP = attP;
        } else {
            userData->result->errors.append(QStringLiteral("no 'p' attribute in %1:%2")
                                            .arg(QDir::toNativeSeparators(*userData->path))
                                            .arg(XML_GetCurrentLineNumber(userData->parser)));
            XML_StopParser(userData->parser, XML_FALSE);
        }
    } else if (strcmp(name, "measValue") == 0) {
        userData->objLdn = findAttribute("measObjLdn", atts);
    } else if (strcmp(name, "suspect") == 0) {
        userData->elemName = xenSuspect;
    } else if (strcmp(name, "measInfo") == 0) {
        userData->measInfoId = getMeasInfoId(atts);
    } else if (strcmp(name, "granPeriod") == 0) {
        const char *endTimeStr = findAttribute("endTime", atts);
        QDateTime endTime = QDateTime::fromString(endTimeStr, Qt::ISODate);
        if (endTime.isValid()) {
            if (endTime != userData->endTime) {
                userData->endTime = endTime;
                userData->result->datas.push_back(MeasData(userData->indexMap->size()));
                userData->result->datas.back().endTime = endTime.toString(DTFMT_IN_CSV).toStdString();
            }
        } else {
            userData->result->errors.append(QStringLiteral("invalid date time format in %1:%2")
                                            .arg(QDir::toNativeSeparators(*userData->path))
                                            .arg(XML_GetCurrentLineNumber(userData->parser)));
            XML_StopParser(userData->parser, XML_FALSE);
        }
    }
}

void KpiKciFileParser::endElement_dataHandler(void *ud, const char *name)
{
    DataUserData *userData = static_cast<DataUserData *>(ud);

    if (strcmp(name, "measType") == 0) {
        userData->elemName = xenNotCare;
        userData->measTypeMap[userData->attP].swap(userData->character);
    } else if (strcmp(name, "r") == 0) {
        userData->elemName = xenNotCare;

        const std::string &measType = userData->measTypeMap[userData->attP];
        std::string counterName = userData->measInfoId;
        counterName += CounterName::sModuleSeparator.toLatin1();
        counterName += userData->objLdn;
        counterName += CounterName::sIndexesSeparator.toLatin1();
        counterName += measType;

        auto iter = userData->indexMap->find(counterName);
        if (iter != userData->indexMap->end()) {
            int index = iter->second;
            userData->indexes.push_back(index);
            userData->result->datas.back().values[index].swap(userData->character);
        } else {
            userData->result->errors.append(QStringLiteral("cannot find counter '%1' in %2")
                                            .arg(QString::fromStdString(counterName),
                                                 QDir::toNativeSeparators(*userData->path)));
            XML_StopParser(userData->parser, XML_FALSE);
        }
    } else if (strcmp(name, "measValue") == 0) {
        if (userData->suspect) {
            userData->suspect = false;
            for (const int &index : userData->indexes) {
                userData->result->datas.back().values[index] += 's';
            }
        }
        userData->indexes.clear();
    } else if (strcmp(name, "suspect") == 0) {
        userData->elemName = xenNotCare;

        if (userData->character == "true") {
            userData->suspect = true;
        }
        userData->character.clear();
    } else if (strcmp(name, "measInfo") == 0) {
        userData->measTypeMap.clear();
    }
}

void KpiKciFileParser::character_dataHandler(void *ud, const char *s, int len)
{
    DataUserData *userData = static_cast<DataUserData *>(ud);

    switch (userData->elemName) {
    case xenMeasType:
    case xenR:
    case xenSuspect:
        userData->character.append(s, len);
        break;
    default:
        break;
    }
}

KpiKciFileParser::DataResult KpiKciFileParser::parseData(const QString &path, const IndexMap &indexMap, volatile const bool &working)
{
    DataResult result;
    GzipFile reader;
    if (!reader.open(path, GzipFile::ReadOnly)) {
        result.errors.append("failed to open ");
        result.errors.last() += QDir::toNativeSeparators(path);
        return result;
    }

    XML_Parser parser = XML_ParserCreate(nullptr);

    DataUserData userData;
    userData.suspect = false;
    userData.elemName = xenNotCare;
    userData.indexMap = &indexMap;
    userData.parser = parser;
    userData.path = &path;
    userData.result = &result;

    XML_SetUserData(parser, &userData);
    XML_SetElementHandler(parser, startElement_dataHandler, endElement_dataHandler);
    XML_SetCharacterDataHandler(parser, character_dataHandler);

    int len;
    char buf[BUFSIZ];
    while (working && result.errors.isEmpty() && (len = reader.read(buf, BUFSIZ)) > 0) {
        int isFinal = len < BUFSIZ;
        if (XML_Parse(parser, buf, len, isFinal) == XML_STATUS_ERROR) {
            result.errors.append(QStringLiteral("failed to parse data in %1:%2: %3")
                                 .arg(QDir::toNativeSeparators(path))
                                 .arg(XML_GetCurrentLineNumber(parser))
                                 .arg(XML_ErrorString(XML_GetErrorCode(parser))));
            break;
        }
    }

    XML_ParserFree(parser);
    return result;
}

void KpiKciFileParser::writeData(DataResult &finalResult, const DataResult &intermResult, GzipFile &writer)
{
    if (intermResult.errors.isEmpty()) {
        for (const MeasData &data : intermResult.datas) {
            writer.write(data.endTime);
            writer.write(";", 1);

            for (auto iter = data.values.begin(); iter != data.values.end() - 1; ++iter) {
                writer.write(*iter);
                writer.write(";", 1);
            }

            writer.write(data.values.back());
            writer.write("\n", 1);
        }
    } else {
        finalResult.errors.append(intermResult.errors);
    }
}
