#include "ColorScheme.h"
#include <QSettings>

ColorScheme &ColorScheme::instance()
{
    static ColorScheme s;
    return s;
}

ColorScheme::ColorScheme()
{
    applyDefaults();
    load();
}

void ColorScheme::applyDefaults()
{
    m_equal          = QColor(200, 230, 201);   // green
    m_different      = QColor(239, 154, 154);   // red
    m_onlyOneSide    = QColor(255, 209, 128);   // amber
    m_missing        = QColor(224, 224, 224);   // grey
    m_outOfRange     = QColor(250, 250, 250);   // off-white
    m_searchHit      = QColor(255, 241, 118);   // yellow
    m_currentHit     = QColor(255, 152,   0);   // orange
    m_offsetBg       = QColor(236, 239, 241);
    m_offsetFg       = QColor( 55,  71,  79);
    m_text           = QColor( 33,  33,  33);
    m_asciiText      = QColor( 69,  90, 100);
    m_viewBackground = QColor(255, 255, 255);
}

void ColorScheme::setColor(QColor &target, const QColor &value, const char *key)
{
    if (!value.isValid() || target == value) return;

    target = value;
    QSettings().setValue(QStringLiteral("colors/") + QString::fromLatin1(key),
                         value.name(QColor::HexArgb));
    emit changed();
}

void ColorScheme::loadColor(QColor &target, const QString &key)
{
    const QVariant v = QSettings().value(QStringLiteral("colors/") + key);
    if (!v.isValid()) return;

    const QColor c(v.toString());
    if (c.isValid()) target = c;
}

void ColorScheme::load()
{
    loadColor(m_equal,          "equal");
    loadColor(m_different,      "different");
    loadColor(m_onlyOneSide,    "onlyOneSide");
    loadColor(m_missing,        "missing");
    loadColor(m_outOfRange,     "outOfRange");
    loadColor(m_searchHit,      "searchHit");
    loadColor(m_currentHit,     "currentHit");
    loadColor(m_offsetBg,       "offsetBg");
    loadColor(m_offsetFg,       "offsetFg");
    loadColor(m_text,           "text");
    loadColor(m_asciiText,      "asciiText");
    loadColor(m_viewBackground, "viewBackground");
}

void ColorScheme::save() const
{
    /* colours are persisted immediately in setColor() — this is a no-op kept
       for API symmetry so future callers can force a flush if needed. */
}

void ColorScheme::resetToDefaults()
{
    QSettings().remove(QStringLiteral("colors"));
    applyDefaults();
    emit changed();
}