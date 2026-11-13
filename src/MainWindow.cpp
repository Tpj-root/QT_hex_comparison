// MainWindow.cpp
#include "MainWindow.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QStatusBar>
#include <QVBoxLayout>
#include <algorithm>

#include "BinaryFile.h"
#include "ByteFormatter.h"
#include "FilePickerWidget.h"
#include "HexDiffModel.h"
#include "HexDiffView.h"
#include "RangeControlWidget.h"
#include "SearchBar.h"
#include "SearchEngine.h"
// ---- includes (top of MainWindow.cpp) add: ----
#include "BookmarkManager.h"
#include "BookmarkWidget.h"
#include <QTabWidget>
#include "SearchBookmarkConverter.h"

#include "ColorScheme.h"          // NEW
#include "ColorSettingsWidget.h"  // NEW
#include <QCheckBox>              // NEW


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Hex Compare — BIN / HEX / ROM binary diff"));
    resize(1440, 880);

    m_leftFile  = new BinaryFile(this);
    m_rightFile = new BinaryFile(this);

    buildUi();
    connectSignals();
    updateStatus();
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *root    = new QVBoxLayout(central);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(6);

    auto *fileRow = new QHBoxLayout;
    fileRow->setSpacing(8);
    m_leftPicker  = new FilePickerWidget(tr("LEFT  (A)"),  this);
    m_rightPicker = new FilePickerWidget(tr("RIGHT (B)"), this);
    fileRow->addWidget(m_leftPicker,  1);
    fileRow->addWidget(m_rightPicker, 1);
    root->addLayout(fileRow);

    m_range = new RangeControlWidget(this);
    root->addWidget(m_range);

    auto *actionRow = new QHBoxLayout;
    m_compareButton  = new QPushButton(tr("⟳ Compare"), this);
    m_prevDiffButton = new QPushButton(tr("◀ Previous difference"), this);
    m_nextDiffButton = new QPushButton(tr("Next difference ▶"), this);
    m_swapButton     = new QPushButton(tr("⇄ Swap sides"), this);
    actionRow->addWidget(m_compareButton);
    actionRow->addWidget(m_prevDiffButton);
    actionRow->addWidget(m_nextDiffButton);
    actionRow->addWidget(m_swapButton);

    // -------- NEW: enable / disable side-by-side comparison --------
    m_enableComparison = new QCheckBox(tr("Enable side-by-side comparison"), this);
    m_enableComparison->setChecked(true);
    m_enableComparison->setToolTip(
        tr("When unchecked, only the LEFT file is shown (no diff computed)."));
    actionRow->addSpacing(16);
    actionRow->addWidget(m_enableComparison);
    // ---------------------------------------------------------------

    actionRow->addStretch(1);
    root->addLayout(actionRow);

    m_search = new SearchBar(this);
    root->addWidget(m_search);

    // ---- Bookmarks manager & model ----
    m_bookmarkManager = new BookmarkManager(this);

    m_model = new HexDiffModel(this);
    m_model->setSources(m_leftFile, m_rightFile);
    m_model->setBookmarkManager(m_bookmarkManager);

    m_view = new HexDiffView(this);
    m_view->setHexModel(m_model);

    m_bookmarkWidget = new BookmarkWidget(m_bookmarkManager, this);

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(m_view,           tr("Hex Compare"));
    m_tabs->addTab(m_bookmarkWidget, tr("Bookmarks"));

    // -------- NEW: colour settings tab --------
    m_colorSettings = new ColorSettingsWidget(this);
    m_tabs->addTab(m_colorSettings,  tr("Colors"));
    // -----------------------------------------

    root->addWidget(m_tabs, 1);
    setCentralWidget(central);

    m_summaryLabel = new QLabel(this);
    m_offsetLabel  = new QLabel(tr("Offset: —"), this);
    statusBar()->addWidget(m_summaryLabel, 1);
    statusBar()->addPermanentWidget(m_offsetLabel);
}

void MainWindow::connectSignals()
{
    connect(m_leftPicker,  &FilePickerWidget::fileChosen, this, &MainWindow::onLeftFileChosen);
    connect(m_rightPicker, &FilePickerWidget::fileChosen, this, &MainWindow::onRightFileChosen);

    connect(m_leftPicker, &FilePickerWidget::cleared, this, [this] {
        m_leftFile->clear(); runComparison();
    });
    connect(m_rightPicker, &FilePickerWidget::cleared, this, [this] {
        m_rightFile->clear(); runComparison();
    });

    connect(m_range, &RangeControlWidget::rangeChanged, this, &MainWindow::onRangeChanged);

    connect(m_compareButton,  &QPushButton::clicked, this, &MainWindow::runComparison);
    connect(m_nextDiffButton, &QPushButton::clicked, this, &MainWindow::goToNextDifference);
    connect(m_prevDiffButton, &QPushButton::clicked, this, &MainWindow::goToPreviousDifference);
    connect(m_swapButton,     &QPushButton::clicked, this, &MainWindow::swapSides);

    connect(m_search, &SearchBar::searchRequested,   this, &MainWindow::runSearch);
    connect(m_search, &SearchBar::nextRequested,     this, &MainWindow::goToNextHit);
    connect(m_search, &SearchBar::previousRequested, this, &MainWindow::goToPreviousHit);
    connect(m_search, &SearchBar::cleared, this, [this] {
        m_hitOffsets.clear();
        m_currentHitIndex = -1;
        m_model->clearSearchHits();
        m_model->refresh();
        m_search->setHitInfo(0, -1);
    });
        // Bookmark tab navigation → switch to hex tab and jump to start
    connect(m_bookmarkWidget, &BookmarkWidget::navigateRequested,
            this, [this](qint64 offset) {
                m_tabs->setCurrentWidget(m_view);
                m_view->goToOffset(offset);
            });

        // --- NEW: search hits → bookmarks ---
    connect(m_search, &SearchBar::bookmarkAllHitsRequested,
            this, &MainWindow::onBookmarkAllHitsRequested);

    // -------- NEW: compare toggle --------
    connect(m_enableComparison, &QCheckBox::toggled,
            this, &MainWindow::onComparisonToggled);

    // -------- NEW: live repaint when any colour is edited --------
    connect(&ColorScheme::instance(), &ColorScheme::changed,
            this, &MainWindow::onColorsChanged);


    // Hex selection → autofill bookmark widget start/end
    connect(m_view, &HexDiffView::selectionRangeChanged,
            m_bookmarkWidget, &BookmarkWidget::setCurrentSelection);

    connect(m_view, &HexDiffView::offsetUnderCursor, this, &MainWindow::onOffsetUnderCursor);
}

void MainWindow::onBookmarkAllHitsRequested(const QColor &color,
                                            const QString &namePrefix)
{
    if (m_hitOffsets.isEmpty() || m_patternLength <= 0) {
        statusBar()->showMessage(
            tr("Nothing to convert — run a search first."), 4000);
        return;
    }

    SearchBookmarkConverter::Options opt;
    opt.color         = color;
    opt.namePrefix    = namePrefix;
    opt.patternLength = m_patternLength;   // set by runSearch()
    opt.skipExisting  = true;              // never overwrite an existing bookmark
    opt.perHitName    = true;              // "DE AD BE EF #1", "#2", …

    const auto res = SearchBookmarkConverter::convert(
        m_bookmarkManager, m_hitOffsets, opt);

    m_model->refresh();                    // pick up new bookmark colours

    if (res.created > 0 && m_tabs && m_bookmarkWidget)
        m_tabs->setCurrentWidget(m_bookmarkWidget);   // show the list

    statusBar()->showMessage(
        tr("Saved %1 hit(s) as bookmarks  ·  %2 skipped (already bookmarked)")
            .arg(res.created).arg(res.skipped),
        6000);
}


void MainWindow::loadFile(BinaryFile *file, FilePickerWidget *picker, const QString &path)
{
    QString error;
    if (!file->load(path, &error)) {
        QMessageBox::warning(this, tr("Cannot open file"), error);
        return;
    }
    picker->setFilePath(path);
    picker->setSizeText(ByteFormatter::humanSize(file->size()));
}

void MainWindow::onLeftFileChosen(const QString &path)
{
    loadFile(m_leftFile, m_leftPicker, path);
    runComparison();
}

void MainWindow::onRightFileChosen(const QString &path)
{
    loadFile(m_rightFile, m_rightPicker, path);
    runComparison();
}

void MainWindow::onRangeChanged(qint64 start, qint64 end, int bytesPerRow)
{
    m_rangeStart  = start;
    m_rangeEnd    = end;
    m_bytesPerRow = bytesPerRow;
    runComparison();
}

void MainWindow::runComparison()
{
    // ---------------------------------------------------------------
    // NEW: comparison disabled → single-file view, no diff, no columns
    // ---------------------------------------------------------------
    if (!m_comparisonEnabled) {
        m_diff = DiffEngine::Result();               // clear any old diff
        m_model->setBytesPerRow(m_bytesPerRow);
        m_model->setDiffResult(m_diff);
        m_model->refresh();
        m_view->refreshMetrics();
        m_view->setRightSideVisible(false);

        m_currentBlock    = -1;
        m_hitOffsets.clear();
        m_currentHitIndex = -1;
        m_search->setHitInfo(0, -1);

        updateStatus();
        return;
    }

    // ---- unchanged original path ----
    if (!m_leftFile->isLoaded() && !m_rightFile->isLoaded()) {
        m_summaryLabel->setText(tr("Select two files to compare."));
        return;
    }

    DiffEngine::Options opt;
    opt.startOffset = m_rangeStart;
    opt.endOffset   = m_rangeEnd;
    opt.mergeGap    = 0;

    m_diff = DiffEngine::compare(m_leftFile->data(), m_rightFile->data(), opt);

    m_model->setBytesPerRow(m_bytesPerRow);
    m_model->setDiffResult(m_diff);
    m_model->clearSearchHits();
    m_model->refresh();

    m_view->refreshMetrics();
    m_view->setRightSideVisible(true);               // ensure visible

    m_currentBlock    = -1;
    m_hitOffsets.clear();
    m_currentHitIndex = -1;

    updateStatus();

    if (m_diff.firstDiff >= 0)
        m_view->goToOffset(m_diff.firstDiff);
}

void MainWindow::updateStatus()
{
    // -------- NEW: single-file view status --------
    if (!m_comparisonEnabled) {
        if (!m_leftFile->isLoaded()) {
            m_summaryLabel->setText(tr("Load a file to view."));
        } else {
            m_summaryLabel->setText(
                tr("Single file view  ·  L: %1  ·  %2 bytes")
                    .arg(ByteFormatter::humanSize(m_leftFile->size()))
                    .arg(m_leftFile->size()));
        }
        return;
    }
    // ---------------------------------------------

    // ---- unchanged original path ----
    const QString leftSize  = m_leftFile->isLoaded()
                                ? ByteFormatter::humanSize(m_leftFile->size())  : tr("—");
    const QString rightSize = m_rightFile->isLoaded()
                                ? ByteFormatter::humanSize(m_rightFile->size()) : tr("—");

    QString text;

    const bool bothLoaded = m_leftFile->isLoaded() && m_rightFile->isLoaded();

    if (!bothLoaded) {
        text = tr("L: %1   |   R: %2   —   load both files to compare")
                   .arg(leftSize, rightSize);
    } else if (m_diff.identical() && m_leftFile->size() == m_rightFile->size()) {
        text = tr("✔ FILES ARE IDENTICAL   ·   L: %1   |   R: %2   ·   %3 bytes compared")
                   .arg(leftSize, rightSize)
                   .arg(m_diff.comparedSize);
    } else {
        const qint64 delta = m_rightFile->size() - m_leftFile->size();
        const QString deltaText = (delta == 0)
            ? tr("equal size")
            : (delta > 0 ? tr("R is %1 larger").arg(ByteFormatter::humanSize(delta))
                         : tr("L is %1 larger").arg(ByteFormatter::humanSize(-delta)));

        text = tr("L: %1  |  R: %2  (%3)   ·   compared %4 bytes   ·   "
                  "%5 different byte(s) in %6 block(s)   ·   first diff @ %7")
                   .arg(leftSize, rightSize, deltaText)
                   .arg(m_diff.comparedSize)
                   .arg(m_diff.diffBytes)
                   .arg(m_diff.blocks.size())
                   .arg(m_diff.firstDiff >= 0
                            ? ByteFormatter::offsetText(m_diff.firstDiff, 8)
                            : tr("—"));
    }

    m_summaryLabel->setText(text);
}

void MainWindow::goToNextDifference()
{
    if (m_diff.blocks.isEmpty()) return;
    m_currentBlock = (m_currentBlock + 1) % m_diff.blocks.size();
    m_view->goToOffset(m_diff.blocks.at(m_currentBlock).start);
}

void MainWindow::goToPreviousDifference()
{
    if (m_diff.blocks.isEmpty()) return;
    m_currentBlock = (m_currentBlock - 1 + m_diff.blocks.size()) % m_diff.blocks.size();
    m_view->goToOffset(m_diff.blocks.at(m_currentBlock).start);
}

void MainWindow::swapSides()
{
    const QString lp = m_leftFile->filePath();
    const QString rp = m_rightFile->filePath();
    if (lp.isEmpty() && rp.isEmpty()) return;

    if (!rp.isEmpty()) loadFile(m_leftFile, m_leftPicker, rp);
    else { m_leftFile->clear(); m_leftPicker->setFilePath(QString());
           m_leftPicker->setSizeText(tr("—")); }

    if (!lp.isEmpty()) loadFile(m_rightFile, m_rightPicker, lp);
    else { m_rightFile->clear(); m_rightPicker->setFilePath(QString());
           m_rightPicker->setSizeText(tr("—")); }

    runComparison();
}

void MainWindow::runSearch()
{
    bool ok = false;
    const QByteArray needle = m_search->pattern(&ok);

    if (!ok || needle.isEmpty()) {
        statusBar()->showMessage(tr("Enter a valid hex byte pattern or ASCII text"), 4000);
        return;
    }

    const SearchEngine::Hits hits =
        SearchEngine::searchBoth(m_leftFile->data(), m_rightFile->data(), needle);

    m_model->setSearchHits(hits.left, hits.right);
    m_model->refresh();

    QSet<qint64> all;
    for (const auto &h : hits.left)  all.insert(h.offset);
    for (const auto &h : hits.right) all.insert(h.offset);

    m_hitOffsets = QVector<qint64>(all.begin(), all.end());
    std::sort(m_hitOffsets.begin(), m_hitOffsets.end());

    m_patternLength   = needle.size();
    m_currentHitIndex = m_hitOffsets.isEmpty() ? -1 : 0;

    m_search->setHitInfo(m_hitOffsets.size(), m_currentHitIndex);

    if (m_currentHitIndex >= 0)
        jumpToHit(0);
    else
        m_model->setCurrentHit(-1, 0);
}

void MainWindow::goToNextHit()
{
    if (m_hitOffsets.isEmpty()) return;
    jumpToHit((m_currentHitIndex + 1) % m_hitOffsets.size());
}

void MainWindow::goToPreviousHit()
{
    if (m_hitOffsets.isEmpty()) return;
    jumpToHit((m_currentHitIndex - 1 + m_hitOffsets.size()) % m_hitOffsets.size());
}

void MainWindow::jumpToHit(int index)
{
    if (m_hitOffsets.isEmpty()) return;

    m_currentHitIndex = (index % m_hitOffsets.size() + m_hitOffsets.size())
                        % m_hitOffsets.size();

    const qint64 off = m_hitOffsets.at(m_currentHitIndex);
    m_model->setCurrentHit(off, m_patternLength);
    m_view->goToOffset(off);
    m_search->setHitInfo(m_hitOffsets.size(), m_currentHitIndex);
}

void MainWindow::onOffsetUnderCursor(qint64 offset)
{
    m_offsetLabel->setText(tr("Offset: %1  (%2)")
                               .arg(ByteFormatter::offsetText(offset, 8))
                               .arg(offset));
}


// ---------------------------------------------------------------------------
// NEW: enable / disable side-by-side comparison
// ---------------------------------------------------------------------------
void MainWindow::onComparisonToggled(bool enabled)
{
    m_comparisonEnabled = enabled;

    // Right picker stays enabled so the user can pre-load a file;
    // only the comparison itself is disabled.
    m_rightPicker->setEnabled(true);

    // Prev / Next difference make no sense without a diff
    m_prevDiffButton->setEnabled(enabled);
    m_nextDiffButton->setEnabled(enabled);

    runComparison();
    m_view->goToOffset(0);
}

// ---------------------------------------------------------------------------
// NEW: repaint hex view when any colour is edited in the Colors tab
// ---------------------------------------------------------------------------
void MainWindow::onColorsChanged()
{
    if (m_model)  m_model->refreshColors();
    if (m_view)   m_view->applyViewBackground();
}