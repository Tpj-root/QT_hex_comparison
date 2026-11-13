#pragma once
#include <QColor>

namespace HexColors {

inline QColor equal()        { return QColor(200, 230, 201); }  // green  – perfect match
inline QColor different()    { return QColor(239, 154, 154); }  // red    – bytes differ
inline QColor onlyOneSide()  { return QColor(255, 209, 128); }  // amber  – one file ended
inline QColor missing()      { return QColor(224, 224, 224); }  // grey   – no data here
inline QColor outOfRange()   { return QColor(250, 250, 250); }  // white  – excluded by range
inline QColor searchHit()    { return QColor(255, 241, 118); }  // yellow – search hit
inline QColor currentHit()   { return QColor(255, 152,   0); }  // orange – active hit
inline QColor offsetBg()     { return QColor(236, 239, 241); }
inline QColor offsetFg()     { return QColor( 55,  71,  79); }
inline QColor text()         { return QColor( 33,  33,  33); }
inline QColor asciiText()    { return QColor( 69,  90, 100); }

} // namespace HexColors