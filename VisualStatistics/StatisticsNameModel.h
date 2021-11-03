#ifndef STATISTICSNAMEMODEL_H
#define STATISTICSNAMEMODEL_H

#include <pcre.h>
#include <QAbstractListModel>

#include <unordered_map>

class StatisticsNameModel : public QAbstractListModel
{
public:
    typedef std::vector<std::string> StatisticsNames;

    struct StatId {
        std::string module;
        std::string group;
        std::string object;

        StatId() {}

        StatId(const std::string &m, const std::string &g, const std::string &o) :
            module(m),
            group(g),
            object(o)
        {
        }

        StatId(StatId &&other) noexcept :
            module(other.module),
            group(other.group),
            object(other.object)
        {
        }

        bool operator==(const StatId &other) const {
            return other.module == module && other.group == group && other.object == object;
        }
    };

    struct StatIdHasher {
        size_t operator()(const StatId &si) const {
            return std::hash<std::string>()(si.module) ^
                    std::hash<std::string>()(si.group) ^
                    std::hash<std::string>()(si.object);
        }
    };

    explicit StatisticsNameModel(QObject *parent);
    StatisticsNameModel(const StatisticsNameModel &) = delete;
    StatisticsNameModel& operator=(const StatisticsNameModel &) = delete;
    ~StatisticsNameModel();

    void setStatisticsNames(StatisticsNames &sns);
    QStringList getModules() const;

    void clearStatisticsNames();

    void setFilterPattern(const QStringList &modules, const QString &pattern, bool caseSensitive, QString &error);
    int matchedCount() const;
    int totalCount() const;
    void parseStatDescription(const QString &path);

    virtual bool canFetchMore(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual void fetchMore(const QModelIndex &parent) Q_DECL_OVERRIDE;
    virtual int	rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    int m_fetchedCount;
    StatisticsNames m_statNames;
    std::vector<int> m_indexes;
    pcre_jit_stack *m_jitStack;

    std::unordered_map<StatId, std::string, StatIdHasher> m_statDesc;
};

#endif // STATISTICSNAMEMODEL_H
