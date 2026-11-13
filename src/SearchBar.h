#pragma once
#include <QByteArray>
#include <QColor>
#include <QWidget>

class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;

class SearchBar : public QWidget
{
    Q_OBJECT
public:
    explicit SearchBar(QWidget *parent = nullptr);

    QByteArray pattern(bool *ok) const;

    void setHitInfo(int total, int currentIndex);

    // --- NEW ---
    QColor  bookmarkColor()      const;
    QString bookmarkNamePrefix() const;
    void    setHitsAvailable(int total);

signals:
    void searchRequested();
    void nextRequested();
    void previousRequested();
    void cleared();

    // --- NEW: fired when the user clicks “Save all hits → Bookmarks” ---
    void bookmarkAllHitsRequested(const QColor &color,
                                  const QString &namePrefix);

private slots:
    void applyBookmarkColorSwatch();
    void onPickBookmarkColor();
    void onBookmarkAllClicked();

private:
    QLineEdit   *m_edit      = nullptr;
    QComboBox   *m_modeCombo = nullptr;
    QPushButton *m_findButton = nullptr;
    QPushButton *m_nextButton = nullptr;
    QPushButton *m_prevButton = nullptr;
    QLabel      *m_hitLabel  = nullptr;

    // --- NEW ---
    QPushButton *m_bookmarkColorButton = nullptr;
    QPushButton *m_bookmarkAllButton   = nullptr;
    QColor       m_bookmarkColor = QColor(255, 193, 7);
};