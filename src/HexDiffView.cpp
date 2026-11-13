#include "HexDiffView.h"

#include <QFontMetrics>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QScrollBar>

#include "HexDiffModel.h"

namespace {
inline int textWidth(const QFontMetrics &fm, const QString &s)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return fm.horizontalAdvance(s);
#else
    return fm.width(s);
#endif
}
inline QPoint eventPos(const QMouseEvent *e)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return e->position().toPoint();
#else
    return e->pos();
#endif
}
} // namespace

HexDiffView::HexDiffView(QWidget *parent) : QTableView(parent)
{
    QFont f = font();
    f.setStyleHint(QFont::Monospace);
    f.setFamily(QStringLiteral("Consolas"));
    f.setFixedPitch(true);
    f.setPointSize(10);
    setFont(f);

    setMouseTracking(true);
    configure();
}

void HexDiffView::configure()
{
    setShowGrid(false);
    setWordWrap(false);
    setAlternatingRowColors(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setCornerButtonEnabled(false);

    verticalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(fontMetrics().height() + 2);

    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setHighlightSections(false);
    horizontalHeader()->setStretchLastSection(false);
}

void HexDiffView::setHexModel(HexDiffModel *model)
{
    if (auto *oldSel = selectionModel())
        disconnect(oldSel, nullptr, this, nullptr);

    m_hexModel = model;
    setModel(model);
    refreshMetrics();

    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &HexDiffView::onSelectionChanged);
}

void HexDiffView::refreshMetrics()
{
    if (!m_hexModel) return;

    const QFontMetrics fm(font());
    const int n          = m_hexModel->bytesPerRow();
    const int hexWidth   = textWidth(fm, QStringLiteral("00")) + 14;
    const int asciiWidth = textWidth(fm, QStringLiteral("W"))  + 12;

    horizontalHeader()->resizeSection(0,
        textWidth(fm, QStringLiteral("00000000:")) + 18);

    for (int i = 0; i < n; ++i) {
        horizontalHeader()->resizeSection(m_hexModel->leftHexColumn(i),    hexWidth);
        horizontalHeader()->resizeSection(m_hexModel->leftAsciiColumn(i),  asciiWidth);
        horizontalHeader()->resizeSection(m_hexModel->rightHexColumn(i),   hexWidth);
        horizontalHeader()->resizeSection(m_hexModel->rightAsciiColumn(i), asciiWidth);
    }
}

void HexDiffView::goToOffset(qint64 offset)
{
    if (!m_hexModel || offset < 0) return;

    const int row = m_hexModel->rowForOffset(offset);
    if (row < 0 || row >= m_hexModel->rowCount()) return;

    const int byteIdx = int(offset - m_hexModel->offsetForRow(row));
    if (byteIdx < 0 || byteIdx >= m_hexModel->bytesPerRow()) return;

    const QModelIndex idx = m_hexModel->index(row, m_hexModel->leftHexColumn(byteIdx));
    setCurrentIndex(idx);
    scrollTo(idx, QAbstractItemView::PositionAtCenter);
    emit offsetUnderCursor(offset);
}

void HexDiffView::mouseMoveEvent(QMouseEvent *event)
{
    QTableView::mouseMoveEvent(event);
    if (!m_hexModel) return;

    const QModelIndex idx = indexAt(eventPos(event));
    if (!idx.isValid() || idx.column() == 0) return;

    const int n   = m_hexModel->bytesPerRow();
    const int col = idx.column();

    int byteIdx = 0;
    if      (col <= n)     byteIdx = col - 1;
    else if (col <= 2 * n) byteIdx = col - n - 1;
    else if (col <= 3 * n) byteIdx = col - 2 * n - 1;
    else                   byteIdx = col - 3 * n - 1;

    emit offsetUnderCursor(m_hexModel->offsetForRow(idx.row()) + byteIdx);
}

void HexDiffView::onSelectionChanged()
{
    if (!m_hexModel) return;

    qint64 minOff = -1;
    qint64 maxOff = -1;
    const int n   = m_hexModel->bytesPerRow();

    for (const auto &range : selectionModel()->selection()) {
        for (int row = range.top(); row <= range.bottom(); ++row) {
            for (int col = range.left(); col <= range.right(); ++col) {
                int byteIdx = -1;
                if      (col >= 1     && col <= n)     byteIdx = col - 1;
                else if (col >= n + 1 && col <= 2 * n) byteIdx = col - n - 1;
                else if (col >= 2*n+1 && col <= 3 * n) byteIdx = col - 2 * n - 1;
                else if (col >= 3*n+1 && col <= 4 * n) byteIdx = col - 3 * n - 1;

                if (byteIdx < 0) continue;
                const qint64 off = m_hexModel->offsetForRow(row) + byteIdx;
                if (minOff < 0 || off < minOff) minOff = off;
                if (maxOff < 0 || off > maxOff) maxOff = off;
            }
        }
    }
    emit selectionRangeChanged(minOff, maxOff);
}