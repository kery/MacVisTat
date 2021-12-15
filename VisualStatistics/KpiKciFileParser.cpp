#include "KpiKciFileParser.h"
#include "ProgressDialog.h"
#include <QtConcurrent>
#include <expat.h>

KpiKciFileParser::KpiKciFileParser(QWidget *parent) :
    mParent(parent)
{
}

QString KpiKciFileParser::convertToCsv(QStringList &paths, QVector<QString> &errors)
{
    ProgressDialog dlg(mParent);
    dlg.setDescription(QStringLiteral("Sorting KPI/KCI files..."));
    dlg.setUndeterminable();
    dlg.setCancelButtonVisible(false);

    QFutureWatcher<void> sortWatcher;
    QObject::connect(&sortWatcher, SIGNAL(finished()), &dlg, SLOT(accept()));

    return QString();
}

void KpiKciFileParser::sortFiles(QStringList &paths, QVector<QString> &failedFiles, QVector<QString> &errors)
{
//    std::sort(paths.begin(), paths.end(), [&failedFiles, &errors](const QString &path1, const QString &paths) -> bool {

//    });
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_ParserFree(parser);
}
