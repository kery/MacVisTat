#ifndef UTILS_H
#define UTILS_H

#include <QMessageBox>

std::string doubleToStdString(double value);

double pointDistance(const QPointF &pt1, const QPointF &pt2);

bool isValidOffsetFromUtc(int offset);

int showQuestionMsgBox(QWidget *parent, const QString &text,
                       QMessageBox::StandardButtons buttons=QMessageBox::Yes|QMessageBox::No,
                       QMessageBox::StandardButton def=QMessageBox::NoButton,
                       const QString &info=QString(),
                       const QString &detail=QString());
void showErrorMsgBox(QWidget *parent, const QString &text,
                     const QString &info=QString(),
                     const QString &detail=QString());

struct lua_State;
QString getLastLuaError(lua_State *L);

#endif // UTILS_H
