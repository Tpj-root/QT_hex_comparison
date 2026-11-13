// HexDiffModel.h
#pragma once
#include <QAbstractTableModel>
#include <QSet>

#include "DiffEngine.h"
#include "SearchEngine.h"

class BinaryFile;

class HexDiffModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column { OffsetColumn = 0 };

    enum Role {
        ByteStateRole = Qt::UserRole + 1,
        SearchHitRole,
        CurrentHitRole,
        SideRole,
        ByteOffsetRole
    };

    enum Side { LeftSide = 0, RightSide = 1 };

    explicit HexDiffModel(QObject *parent = nullptr);

    // setters only store data — call refresh() once afterwards
    void setSources(BinaryFile *left, BinaryFile *right);
    void setDiffResult(const DiffEngine::Result &result);
    void setBytesPerRow(int bytes);

    void setSearchHits(const QVector<SearchEngine::Hit> &leftHits,
                       const QVector<SearchEngine::Hit> &rightHits);
    void setCurrentHit(qint64 offset, int length);
    void clearSearchHits();

    void refresh();

    // QAbstractTableModel
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int      columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // layout helpers
    int    bytesPerRow() const { return m_bytesPerRow; }
    qint64 offsetForRow(int row) const { return qint64(row) * m_bytesPerRow; }
    int    rowForOffset(qint64 offset) const;
    qint64 maxSize() const;
    qint64 totalRows() const { return rowCount(); }

    int leftHexColumn(int i)    const { return 1 + i; }
    int leftAsciiColumn(int i)  const { return 1 + m_bytesPerRow + i; }
    int rightHexColumn(int i)   const { return 1 + 2 * m_bytesPerRow + i; }
    int rightAsciiColumn(int i) const { return 1 + 3 * m_bytesPerRow + i; }

private:
    static QColor backgroundFor(ByteState state);
    ByteState     stateFor(Side side, qint64 offset) const;
    quint8        byteAt(Side side, qint64 offset, bool *exists) const;
    void          emitRangeChanged(qint64 offset, int length);

    BinaryFile        *m_left  = nullptr;
    BinaryFile        *m_right = nullptr;
    DiffEngine::Result m_diff;
    int                m_bytesPerRow  = 16;
    int                m_offsetDigits = 8;

    QSet<qint64> m_leftHitBytes;
    QSet<qint64> m_rightHitBytes;

    qint64 m_currentHitOffset = -1;
    int    m_currentHitLength = 0;
};