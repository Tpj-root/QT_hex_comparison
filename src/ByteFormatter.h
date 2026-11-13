// ByteFormatter.h
#pragma once
#include <QByteArray>
#include <QString>

class ByteFormatter
{
public:
    static QString hexByte(quint8 value);                       // "AF"
    static QString asciiChar(quint8 value);                     // "a" or "."
    static QString offsetText(qint64 offset, int digits = 8);   // "00000010:"
    static int     offsetDigits(qint64 maxSize);                // 8 or 16
    static QString humanSize(qint64 bytes);                     // "1.25 MB"

    static bool       parseOffset(const QString &text, qint64 *value);   // hex by default
    static QByteArray parseBytePattern(const QString &text, bool *ok);   // auto hex/ascii
};