#pragma once
#include <QColor>
#include <QObject>

/**
 * Application-wide colour palette.
 *
 * Every colour used by the hex view is stored here, is persisted
 * in QSettings, and emits changed() whenever any colour is
 * updated so the whole UI can repaint live.
 */
class ColorScheme : public QObject
{
    Q_OBJECT
public:
    static ColorScheme &instance();

    // ---- Diff backgrounds ----
    QColor equal()        const { return m_equal; }
    QColor different()    const { return m_different; }
    QColor onlyOneSide()  const { return m_onlyOneSide; }
    QColor missing()      const { return m_missing; }
    QColor outOfRange()   const { return m_outOfRange; }

    // ---- Search highlights ----
    QColor searchHit()    const { return m_searchHit; }
    QColor currentHit()   const { return m_currentHit; }

    // ---- Offset column ----
    QColor offsetBg()     const { return m_offsetBg; }
    QColor offsetFg()     const { return m_offsetFg; }

    // ---- Text ----
    QColor text()         const { return m_text; }
    QColor asciiText()    const { return m_asciiText; }

    // ---- View background (empty area behind cells) ----
    QColor viewBackground() const { return m_viewBackground; }

    // ---- Setters (all emit changed()) ----
    void setEqual(const QColor &c)        { setColor(m_equal,        c, "equal"); }
    void setDifferent(const QColor &c)    { setColor(m_different,    c, "different"); }
    void setOnlyOneSide(const QColor &c)  { setColor(m_onlyOneSide,  c, "onlyOneSide"); }
    void setMissing(const QColor &c)      { setColor(m_missing,      c, "missing"); }
    void setOutOfRange(const QColor &c)   { setColor(m_outOfRange,   c, "outOfRange"); }
    void setSearchHit(const QColor &c)    { setColor(m_searchHit,    c, "searchHit"); }
    void setCurrentHit(const QColor &c)   { setColor(m_currentHit,   c, "currentHit"); }
    void setOffsetBg(const QColor &c)     { setColor(m_offsetBg,     c, "offsetBg"); }
    void setOffsetFg(const QColor &c)     { setColor(m_offsetFg,     c, "offsetFg"); }
    void setText(const QColor &c)         { setColor(m_text,         c, "text"); }
    void setAsciiText(const QColor &c)    { setColor(m_asciiText,    c, "asciiText"); }
    void setViewBackground(const QColor &c) { setColor(m_viewBackground, c, "viewBackground"); }

    void save() const;
    void load();
    void resetToDefaults();

signals:
    void changed();

private:
    ColorScheme();
    void setColor(QColor &target, const QColor &value, const char *key);
    void loadColor(QColor &target, const QString &key);
    void applyDefaults();

    QColor m_equal;
    QColor m_different;
    QColor m_onlyOneSide;
    QColor m_missing;
    QColor m_outOfRange;
    QColor m_searchHit;
    QColor m_currentHit;
    QColor m_offsetBg;
    QColor m_offsetFg;
    QColor m_text;
    QColor m_asciiText;
    QColor m_viewBackground;
};