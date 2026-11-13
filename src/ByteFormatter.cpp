// ByteFormatter.cpp
#include "ByteFormatter.h"
#include <QRegularExpression>
#include <QStringList>

QString ByteFormatter::hexByte(quint8 value)
{
    return QStringLiteral("%1").arg(value, 2, 16, QChar('0')).toUpper();
}

QString ByteFormatter::asciiChar(quint8 value)
{
    return (value >= 0x20 && value < 0x7F) ? QString(QChar(value)) : QStringLiteral(".");
}

QString ByteFormatter::offsetText(qint64 offset, int digits)
{
    return QStringLiteral("%1:").arg(offset, digits, 16, QChar('0')).toUpper();
}

int ByteFormatter::offsetDigits(qint64 maxSize)
{
    if (maxSize <= 0) return 8;
    return (maxSize <= 0xFFFFFFFFLL) ? 8 : 16;
}

QString ByteFormatter::humanSize(qint64 bytes)
{
    const double b = double(bytes);
    if (bytes < 1024)                    return QStringLiteral("%1 B").arg(bytes);
    if (bytes < 1024LL * 1024)           return QStringLiteral("%1 KB").arg(b / 1024.0, 0, 'f', 2);
    if (bytes < 1024LL * 1024 * 1024)    return QStringLiteral("%1 MB").arg(b / (1024.0 * 1024.0), 0, 'f', 2);
    return QStringLiteral("%1 GB").arg(b / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
}

bool ByteFormatter::parseOffset(const QString &text, qint64 *value)
{
    QString t = text.trimmed();
    if (t.isEmpty()) return false;
    t.remove(QLatin1Char(' '));
    t.remove(QLatin1Char('_'));

    bool decimal = false;
    if (t.startsWith(QLatin1String("0d"), Qt::CaseInsensitive)) { decimal = true; t = t.mid(2); }
    else if (t.startsWith(QLatin1String("0x"), Qt::CaseInsensitive)) { t = t.mid(2); }
    else if (t.startsWith(QLatin1Char('#'))) { t = t.mid(1); }

    bool ok = false;
    const qlonglong v = t.toLongLong(&ok, decimal ? 10 : 16);
    if (!ok || v < 0) return false;
    if (value) *value = v;
    return true;
}

QByteArray ByteFormatter::parseBytePattern(const QString &text, bool *ok)
{
    if (ok) *ok = false;
    QString t = text.trimmed();
    if (t.isEmpty()) return {};

    t.replace(QLatin1String("\\x"), QLatin1String(" "));
    t.replace(QLatin1String("0x"),  QLatin1String(" "));
    t.replace(QLatin1Char(','), QLatin1Char(' '));
    t.replace(QLatin1Char(';'), QLatin1Char(' '));

    const QStringList parts = t.split(QRegularExpression(QStringLiteral("\\s+")),
                                      Qt::SkipEmptyParts);

    bool allHex = !parts.isEmpty();
    for (const QString &p : parts) {
        bool good = false;
        const uint v = p.toUInt(&good, 16);
        if (!good || v > 0xFF || p.size() > 2) { allHex = false; break; }
    }

    if (allHex) {
        QByteArray out;
        for (const QString &p : parts) out.append(char(p.toUInt(nullptr, 16)));
        if (ok) *ok = true;
        return out;
    }

    const QByteArray ascii = text.toLatin1();   // fallback: literal ASCII
    if (ok) *ok = !ascii.isEmpty();
    return ascii;
}