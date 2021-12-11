#ifndef SCILEXERLUA5_2_H
#define SCILEXERLUA5_2_H

#include <Qsci/qscilexerlua.h>

class SciLexerLua5_2 : public QsciLexerLua
{
    Q_OBJECT

public:
    enum {
        Bit32DebugPackageFunctions = KeywordSet5,
        PlotFunctions = KeywordSet6
    };

    SciLexerLua5_2(QObject *parent);

    virtual const char * keywords(int set) const override;
    virtual QFont defaultFont(int style) const override;
    virtual QColor defaultPaper(int style) const override;
};

#endif // SCILEXERLUA5_2_H
