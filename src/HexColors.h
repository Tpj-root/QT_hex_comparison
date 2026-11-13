#pragma once
#include <QColor>
#include "ColorScheme.h"

/**
 * Backward-compatible facade over ColorScheme.
 * All previous HexColors::xxx() call sites keep working unchanged,
 * but every colour now reflects the live, user-editable palette.
 */
namespace HexColors {

inline QColor equal()        { return ColorScheme::instance().equal(); }
inline QColor different()    { return ColorScheme::instance().different(); }
inline QColor onlyOneSide()  { return ColorScheme::instance().onlyOneSide(); }
inline QColor missing()      { return ColorScheme::instance().missing(); }
inline QColor outOfRange()   { return ColorScheme::instance().outOfRange(); }
inline QColor searchHit()    { return ColorScheme::instance().searchHit(); }
inline QColor currentHit()   { return ColorScheme::instance().currentHit(); }
inline QColor offsetBg()     { return ColorScheme::instance().offsetBg(); }
inline QColor offsetFg()     { return ColorScheme::instance().offsetFg(); }
inline QColor text()         { return ColorScheme::instance().text(); }
inline QColor asciiText()    { return ColorScheme::instance().asciiText(); }

} // namespace HexColors