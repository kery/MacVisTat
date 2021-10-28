#include "SciLexerLua5_2.h"

SciLexerLua5_2::SciLexerLua5_2(QObject *parent) :
    QsciLexerLua(parent)
{
}

SciLexerLua5_2::~SciLexerLua5_2()
{
}

const char * SciLexerLua5_2::keywords(int set) const
{
    if (set == 1) {
        // Keywords.
        return
            "and break do else elseif end false for function goto "
            "if in local nil not or repeat return then true until "
            "while";
    }

    if (set == 2) {
        // Basic functions.
        return
            "_G _VERSION assert collectgarbage dofile error "
            "getmetatable ipairs load loadfile next pairs pcall "
            "print rawequal rawget rawlen rawset require select "
            "setmetatable tonumber tostring type xpcall";
    }

    if (set == 3) {
        // String, table and maths functions.
        return
            "string.byte string.char string.dump string.find string.format "
            "string.gmatch string.gsub string.len string.lower string.match "
            "string.rep string.reverse string.sub string.upper "

            "table.concat table.insert table.pack table.remove table.sort "
            "table.unpack "

            "math.abs math.acos math.asin math.atan math.atan2 math.ceil "
            "math.cos math.cosh math.deg math.exp math.floor math.fmod "
            "math.frexp math.huge math.ldexp math.log math.max math.min "
            "math.modf math.pi math.pow math.rad math.random math.randomseed "
            "math.sin math.sinh math.sqrt math.tan math.tanh";
    }

    if (set == 4) {
        // Coroutine, I/O and system facilities.
        return
            "coroutine.create coroutine.resume coroutine.running coroutine.status "
            "coroutine.wrap coroutine.yield "

            "io.close io.flush io.input io.lines io.open io.output io.popen io.read "
            "io.stderr io.stdin io.stdout io.tmpfile io.type io.write close flush "
            "lines read seek setvbuf write "

            "os.clock os.date os.difftime os.execute os.exit os.getenv os.remove "
            "os.rename os.setlocale os.time os.tmpname";
    }

    if (set == 5) {
        // bit32, debug and package functions
        return
            "bit32.arshift bit32.band bit32.bnot bit32.bor bit32.btest bit32.bxor "
            "bit32.extract bit32.lrotate bit32.lshift bit32.replace bit32.rrotate "
            "bit32.rshift "

            "debug.debug debug.getuservalue debug.gethook debug.getinfo debug.getlocal "
            "debug.getmetatable debug.getregistry debug.getupvalue debug.setuservalue "
            "debug.sethook debug.setlocal debug.setmetatable debug.setupvalue "
            "debug.traceback debug.upvalueid debug.upvaluejoin "

            "package.config package.cpath package.loaded package.loadlib package.path "
            "package.preload package.searchers package.searchpath";
    }

    if (set == 6) {
        // plot functions
        return
            "plot.graph_count plot.graph_name plot.get_lastkey plot.get_value plot.get_dt "
            "plot.get_dtstr plot.set_selected plot.add_graph plot.update";
    }

    return 0;
}

QFont SciLexerLua5_2::defaultFont(int style) const
{
    QFont font = QsciLexer::defaultFont(style);

    switch (style) {
    case Comment:
    case LineComment:
        font.setStyle(QFont::StyleItalic);
        break;
    }

    return font;
}

QColor SciLexerLua5_2::defaultPaper(int style) const
{
    if (style == UnclosedString) {
        return QColor(0xe0, 0xc0, 0xe0);
    }
    return QsciLexer::defaultPaper(style);
}
