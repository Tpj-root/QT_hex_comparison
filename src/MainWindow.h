#pragma once
#include <QMainWindow>
#include <QVector>

#include "DiffEngine.h"

class BinaryFile;
class BookmarkManager;      //  <-- new
class BookmarkWidget;       //  <-- new
class HexDiffModel;
class HexDiffView;
class FilePickerWidget;
class RangeControlWidget;
class SearchBar;
class QLabel;
class QPushButton;
class QTabWidget;           //  <-- new

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onLeftFileChosen(const QString &path);
    void onRightFileChosen(const QString &path);
    void onRangeChanged(qint64 start, qint64 end, int bytesPerRow);

    void runComparison();
    void goToNextDifference();
    void goToPreviousDifference();
    void swapSides();

    void runSearch();
    void goToNextHit();
    void goToPreviousHit();

    void onOffsetUnderCursor(qint64 offset);

    // NEW
    void onBookmarkAllHitsRequested(const QColor &color,
                                    const QString &namePrefix);

private:
    void buildUi();
    void connectSignals();
    void updateStatus();
    void loadFile(BinaryFile *file, FilePickerWidget *picker, const QString &path);
    void jumpToHit(int index);

    BinaryFile *m_leftFile  = nullptr;
    BinaryFile *m_rightFile = nullptr;

    FilePickerWidget   *m_leftPicker  = nullptr;
    FilePickerWidget   *m_rightPicker = nullptr;
    RangeControlWidget *m_range       = nullptr;
    SearchBar          *m_search      = nullptr;
    HexDiffView        *m_view        = nullptr;
    HexDiffModel       *m_model       = nullptr;

    QTabWidget         *m_tabs           = nullptr;   //  <-- new
    BookmarkManager    *m_bookmarkManager = nullptr;  //  <-- new
    BookmarkWidget     *m_bookmarkWidget = nullptr;   //  <-- new

    QPushButton *m_compareButton  = nullptr;
    QPushButton *m_prevDiffButton = nullptr;
    QPushButton *m_nextDiffButton = nullptr;
    QPushButton *m_swapButton     = nullptr;

    QLabel *m_summaryLabel = nullptr;
    QLabel *m_offsetLabel  = nullptr;

    DiffEngine::Result m_diff;
    int    m_currentBlock = -1;

    QVector<qint64> m_hitOffsets;
    int    m_currentHitIndex  = -1;
    int    m_patternLength    = 0;

    qint64 m_rangeStart   = 0;
    qint64 m_rangeEnd     = -1;
    int    m_bytesPerRow  = 16;
};