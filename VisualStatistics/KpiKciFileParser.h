#ifndef KPIKCIFILEPARSER_H
#define KPIKCIFILEPARSER_H

#include <QVector>
#include <QDateTime>
#include <map>
#include <set>
#include <unordered_map>
#include <expat.h>

class QWidget;
class ProgressDialog;
class GzipFile;

class KpiKciFileParser
{
public:
    KpiKciFileParser(QWidget *parent);

    QString convertToCsv(QVector<QString> &paths, QVector<QString> &errors);

private:
    typedef std::map<std::string, std::set<std::string>> ObjLdnMap;
    typedef std::map<std::string, ObjLdnMap> InfoIdMap;
    typedef std::unordered_map<std::string, int> IndexMap;
    typedef std::unordered_map<std::string, std::string> MeasTypeMap;

    struct HeaderResult
    {
        InfoIdMap infoIdMap;
        QVector<QString> errors;
        QVector<QString> paths;

        HeaderResult();
    };

    enum XmlElementName
    {
        xenNotCare,
        xenMeasType,
        xenR,
        xenSuspect,
    };

    struct HeaderUserData
    {
        XmlElementName elemName;
        std::string measInfoId;
        std::string character;
        std::vector<std::string> measTypes;
        XML_Parser parser;
        const QString *path;
        HeaderResult *result;
    };

    struct MeasData
    {
        std::string endTime;
        std::vector<std::string> values;

        MeasData(size_t size);
    };

    struct DataResult
    {
        std::vector<MeasData> datas;
        QVector<QString> errors;

        DataResult();
    };

    struct DataUserData
    {
        bool suspect;
        XmlElementName elemName;
        QDateTime endTime;
        std::string measInfoId;
        std::string attP;
        std::string objLdn;
        std::string character;
        MeasTypeMap measTypeMap;
        std::vector<int> indexes;
        const IndexMap *indexMap;
        XML_Parser parser;
        const QString *path;
        DataResult *result;
    };

    static const char * findAttribute(const char *name, const char **atts);
    static QString getAttribute(const QString &path, XML_StartElementHandler handler);
    static const char * getMeasInfoId(const char **atts);
    static std::string getOffsetFromUtc(const QString &path);
    static QString toIsoDateFormat(QString &&dateTime, const QStringRef &offsetFromUtc);
    static void getBeginTime_handler(void *ud, const char *name, const char **atts);
    static QString getBeginTime(const QString &path);
    static void getEndTime_handler(void *ud, const char *name, const char **atts);
    static QString getEndTime(const QString &path);
    static void sortFiles(QVector<QString> &paths, QVector<QString> &errors);
    static void getManagedElement_handler(void *ud, const char *name, const char **atts);
    static QString getManagedElement(const QString &path);
    static QString getUniqueIdFromFileName(const QString &path);
    static void getFirstGranPeriodEndTime_handler(void *ud, const char *name, const char **atts);
    static QString getFirstGranPeriodEndTime(const QString &path);
    static QString getOutputPath(const QVector<QString> &paths);
    static void startElement_headerHandler(void *ud, const char *name, const char **atts);
    static void endElement_headerHandler(void *ud, const char *name);
    static void character_headerHandler(void *ud, const char *s, int len);
    static void mergeHeaderResult(HeaderResult &finalResult, const HeaderResult &intermResult);
    static HeaderResult parseHeader(const QString &path, volatile const bool &working);
    static void writeHeader(const InfoIdMap &iim, const std::string &offsetFromUtc, volatile const bool &working,
                            ProgressDialog &dlg, GzipFile &writer, IndexMap &indexMap);
    static void startElement_dataHandler(void *ud, const char *name, const char **atts);
    static void endElement_dataHandler(void *ud, const char *name);
    static void character_dataHandler(void *ud, const char *s, int len);
    static DataResult parseData(const QString &path, const IndexMap &indexMap, volatile const bool &working);
    static void writeData(DataResult &finalResult, const DataResult &intermResult, GzipFile &writer);

    static QRegularExpression mRegExpTypeA, mRegExpTypeC;

    QWidget *mParent;
};

#endif // KPIKCIFILEPARSER_H
