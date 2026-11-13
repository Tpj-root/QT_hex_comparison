#include "HexDiffModel.h"

#include "BinaryFile.h"
#include "Bookmark.h"
#include "BookmarkManager.h"
#include "ByteFormatter.h"
#include "HexColors.h"

HexDiffModel::HexDiffModel(QObject *parent) : QAbstractTableModel(parent) {}

void HexDiffModel::setSources(BinaryFile *left, BinaryFile *right)
{
    m_left  = left;
    m_right = right;
}

void HexDiffModel::setDiffResult(const DiffEngine::Result &result) { m_diff = result; }

void HexDiffModel::setBytesPerRow(int bytes)
{
    m_bytesPerRow = qBound(1, bytes, 64);
}

void HexDiffModel::setBookmarkManager(BookmarkManager *manager)
{
    if (m_bookmarks)
        disconnect(m_bookmarks, nullptr, this, nullptr);

    m_bookmarks = manager;

    if (m_bookmarks) {
        connect(m_bookmarks, &BookmarkManager::changed, this, [this] {
            if (rowCount() > 0) {
                emit dataChanged(index(0, 0),
                                 index(rowCount() - 1, columnCount() - 1),
                                 { Qt::BackgroundRole, Qt::ToolTipRole, BookmarkRole });
            }
        });
    }
}

void HexDiffModel::setSearchHits(const QVector<SearchEngine::Hit> &leftHits,
                                 const QVector<SearchEngine::Hit> &rightHits)
{
    m_leftHitBytes.clear();
    m_rightHitBytes.clear();

    for (const auto &h : leftHits)
        for (int i = 0; i < h.length; ++i) m_leftHitBytes.insert(h.offset + i);
    for (const auto &h : rightHits)
        for (int i = 0; i < h.length; ++i) m_rightHitBytes.insert(h.offset + i);
}

void HexDiffModel::setCurrentHit(qint64 offset, int length)
{
    const qint64 oldOffset = m_currentHitOffset;
    const int    oldLength = m_currentHitLength;
    m_currentHitOffset = offset;
    m_currentHitLength = length;

    if (oldOffset >= 0) emitRangeChanged(oldOffset, oldLength);
    if (offset    >= 0) emitRangeChanged(offset,    length);
}

void HexDiffModel::clearSearchHits()
{
    m_leftHitBytes.clear();
    m_rightHitBytes.clear();
    m_currentHitOffset = -1;
    m_currentHitLength = 0;
}

void HexDiffModel::refresh()
{
    beginResetModel();
    m_offsetDigits = ByteFormatter::offsetDigits(maxSize());
    endResetModel();
}

int HexDiffModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    const qint64 max = maxSize();
    if (max <= 0) return 0;
    return int((max + m_bytesPerRow - 1) / m_bytesPerRow);
}

int HexDiffModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return 4 * m_bytesPerRow + 1;
}

qint64 HexDiffModel::maxSize() const
{
    qint64 m = 0;
    if (m_left)  m = qMax(m, m_left->size());
    if (m_right) m = qMax(m, m_right->size());
    if (m_diff.maxSize > m) m = m_diff.maxSize;
    return m;
}

int HexDiffModel::rowForOffset(qint64 offset) const
{
    if (offset < 0) return -1;
    return int(offset / m_bytesPerRow);
}

QColor HexDiffModel::backgroundFor(ByteState s)
{
    switch (s) {
    case ByteState::Equal:      return HexColors::equal();
    case ByteState::Different:  return HexColors::different();
    case ByteState::OnlyLeft:
    case ByteState::OnlyRight:  return HexColors::onlyOneSide();
    case ByteState::Missing:    return HexColors::missing();
    case ByteState::OutOfRange: return HexColors::outOfRange();
    case ByteState::Empty:      return HexColors::outOfRange();
    }
    return Qt::white;
}

ByteState HexDiffModel::stateFor(Side side, qint64 offset) const
{
    const QVector<ByteState> &v =
        (side == LeftSide) ? m_diff.leftStates : m_diff.rightStates;
    if (offset < 0 || offset >= v.size()) return ByteState::Empty;
    return v.at(int(offset));
}

quint8 HexDiffModel::byteAt(Side side, qint64 offset, bool *exists) const
{
    const BinaryFile *f = (side == LeftSide) ? m_left : m_right;
    if (f && offset >= 0 && offset < f->size()) {
        if (exists) *exists = true;
        return quint8(f->data().at(int(offset)));
    }
    if (exists) *exists = false;
    return 0;
}

const Bookmark *HexDiffModel::bookmarkAt(qint64 offset) const
{
    if (!m_bookmarks || m_bookmarks->count() == 0) return nullptr;
    return m_bookmarks->bookmarkAt(offset);
}

QVariant HexDiffModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    const int    row  = index.row();
    const int    col  = index.column();
    const qint64 base = offsetForRow(row);
    const int    n    = m_bytesPerRow;

    if (col == OffsetColumn) {
        switch (role) {
        case Qt::DisplayRole:       return ByteFormatter::offsetText(base, m_offsetDigits);
        case Qt::BackgroundRole:    return HexColors::offsetBg();
        case Qt::ForegroundRole:    return HexColors::offsetFg();
        case Qt::TextAlignmentRole: return int(Qt::AlignRight | Qt::AlignVCenter);
        default:                    return QVariant();
        }
    }

    Side side    = LeftSide;
    bool isAscii = false;
    int  byteIdx = 0;

    if      (col <= n)      { side = LeftSide;  isAscii = false; byteIdx = col - 1; }
    else if (col <= 2 * n)  { side = LeftSide;  isAscii = true;  byteIdx = col - n - 1; }
    else if (col <= 3 * n)  { side = RightSide; isAscii = false; byteIdx = col - 2 * n - 1; }
    else                    { side = RightSide; isAscii = true;  byteIdx = col - 3 * n - 1; }

    const qint64 off = base + byteIdx;
    bool         exists = false;
    const quint8 value  = byteAt(side, off, &exists);
    const ByteState st  = stateFor(side, off);

    const bool inCurrentHit =
        (m_currentHitOffset >= 0 && off >= m_currentHitOffset &&
         off < m_currentHitOffset + m_currentHitLength);

    const QSet<qint64> &hitSet = (side == LeftSide) ? m_leftHitBytes : m_rightHitBytes;

    const Bookmark *bm = bookmarkAt(off);

    switch (role) {
    case Qt::DisplayRole:
        if (!exists) return QString();
        return isAscii ? ByteFormatter::asciiChar(value) : ByteFormatter::hexByte(value);

    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);

    case Qt::ForegroundRole:
        return isAscii ? HexColors::asciiText() : HexColors::text();

    case Qt::BackgroundRole:
        if (inCurrentHit)         return HexColors::currentHit();   // highest
        if (hitSet.contains(off)) return HexColors::searchHit();
        if (bm)                   return bm->color;                 // bookmark overrides diff
        return backgroundFor(st);

    case Qt::ToolTipRole: {
        if (!bm) return QVariant();
        QString tip = QStringLiteral("<b>%1</b>").arg(bm->name.toHtmlEscaped());
        tip += QStringLiteral("<br>Start: <code>0x%1</code>")
                   .arg(bm->start, 8, 16, QChar('0'));
        tip += QStringLiteral("<br>End:&nbsp;&nbsp; <code>0x%1</code>")
                   .arg(bm->end - 1, 8, 16, QChar('0'));
        tip += QStringLiteral("<br>Size:  %1 bytes").arg(bm->length());
        tip += QStringLiteral("<br>Offset: <code>0x%1</code>")
                   .arg(off, 8, 16, QChar('0'));
        if (exists) {
            tip += QStringLiteral("<br>Byte:  <code>%1</code> (%2)")
                       .arg(ByteFormatter::hexByte(value),
                            ByteFormatter::asciiChar(value).toHtmlEscaped());
        }
        return tip;
    }

    case ByteStateRole:    return int(st);
    case SearchHitRole:    return hitSet.contains(off);
    case CurrentHitRole:   return inCurrentHit;
    case SideRole:         return int(side);
    case ByteOffsetRole:   return QVariant::fromValue(off);
    case BookmarkRole:     return bm ? bm->id : -1;       //  <-- new
    default:               return QVariant();
    }
}

QVariant HexDiffModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return (role == Qt::DisplayRole) ? QVariant(section) : QVariant();

    if (role != Qt::DisplayRole) return QVariant();
    if (section == OffsetColumn) return tr("Offset");

    const int n = m_bytesPerRow;
    if (section <= n)
        return QStringLiteral("%1").arg(section - 1, 2, 16, QChar('0')).toUpper();
    if (section <= 2 * n)
        return (section == n + 1) ? tr("ASCII") : QString();
    if (section <= 3 * n)
        return QStringLiteral("%1").arg(section - 2 * n - 1, 2, 16, QChar('0')).toUpper();
    return (section == 3 * n + 1) ? tr("ASCII") : QString();
}

Qt::ItemFlags HexDiffModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void HexDiffModel::emitRangeChanged(qint64 offset, int length)
{
    if (offset < 0 || length <= 0) return;

    const int firstRow = rowForOffset(offset);
    const int lastRow  = rowForOffset(offset + length - 1);
    if (firstRow < 0 || lastRow < firstRow) return;
    if (lastRow >= rowCount()) return;

    emit dataChanged(index(firstRow, 0),
                     index(lastRow, columnCount() - 1));
}