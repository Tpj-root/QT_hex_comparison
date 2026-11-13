// DiffEngine.h
#pragma once
#include <QByteArray>
#include <QVector>

enum class ByteState : quint8 {
    Empty = 0,   // no data on either side
    Equal,       // identical
    Different,   // both have data, bytes differ
    OnlyLeft,    // left has data, right ended
    OnlyRight,   // right has data, left ended
    Missing,     // this side has no data but the other does
    OutOfRange   // excluded by the active address range
};

inline bool isDiffState(ByteState s)
{
    return s != ByteState::Equal && s != ByteState::Empty && s != ByteState::OutOfRange;
}

struct DiffBlock { qint64 start = 0; qint64 length = 0; };

class DiffEngine
{
public:
    struct Options {
        qint64 startOffset;
        qint64 endOffset;
        qint64 mergeGap;

        Options()
            : startOffset(0),
              endOffset(-1),
              mergeGap(0)
        {
        }
    };
    struct Result {
        QVector<ByteState> leftStates;
        QVector<ByteState> rightStates;
        QVector<DiffBlock> blocks;
        qint64 maxSize      = 0;
        qint64 comparedSize = 0;
        qint64 equalBytes   = 0;
        qint64 diffBytes    = 0;
        qint64 firstDiff    = -1;

        bool identical() const { return diffBytes == 0 && blocks.isEmpty(); }
    };

    static Result compare(const QByteArray &left,
                          const QByteArray &right,
                          const Options &opt = Options());
};