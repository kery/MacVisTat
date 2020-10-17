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
    virtual ~SciLexerLua5_2();

    virtual const char * keywords(int set) const;
    virtual QFont defaultFont(int style) const;
    virtual QColor defaultPaper(int style) const;
};

#endif // SCILEXERLUA5_2_H
