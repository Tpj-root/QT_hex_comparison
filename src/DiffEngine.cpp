// DiffEngine.cpp
#include "DiffEngine.h"
#include <QtGlobal>
#include <limits>

DiffEngine::Result DiffEngine::compare(const QByteArray &left,
                                       const QByteArray &right,
                                       const Options &opt)
{
    Result r;
    const qint64 rawMax = qMax<qint64>(left.size(), right.size());
    const int    n      = int(qMin<qint64>(rawMax, std::numeric_limits<int>::max()));
    r.maxSize = rawMax;

    r.leftStates.fill(ByteState::Empty, n);
    r.rightStates.fill(ByteState::Empty, n);

    qint64 start = qMax<qint64>(0, opt.startOffset);
    if (start > n) start = n;

    qint64 end = opt.endOffset;
    if (end < 0 || end > n) end = n;
    if (end < start) end = start;

    r.comparedSize = end - start;

    for (int i = 0; i < n; ++i) {
        const qint64 off = i;

        if (off < start || off >= end) {
            r.leftStates[i]  = ByteState::OutOfRange;
            r.rightStates[i] = ByteState::OutOfRange;
            continue;
        }

        const bool hasL = off < left.size();
        const bool hasR = off < right.size();

        if (hasL && hasR) {
            if (left.at(int(off)) == right.at(int(off))) {
                r.leftStates[i] = ByteState::Equal;
                r.rightStates[i] = ByteState::Equal;
                ++r.equalBytes;
            } else {
                r.leftStates[i] = ByteState::Different;
                r.rightStates[i] = ByteState::Different;
                ++r.diffBytes;
                if (r.firstDiff < 0) r.firstDiff = off;
            }
        } else if (hasL) {
            r.leftStates[i]  = ByteState::OnlyLeft;
            r.rightStates[i] = ByteState::Missing;
            ++r.diffBytes;
            if (r.firstDiff < 0) r.firstDiff = off;
        } else if (hasR) {
            r.leftStates[i]  = ByteState::Missing;
            r.rightStates[i] = ByteState::OnlyRight;
            ++r.diffBytes;
            if (r.firstDiff < 0) r.firstDiff = off;
        } else {
            r.leftStates[i]  = ByteState::Empty;
            r.rightStates[i] = ByteState::Empty;
        }
    }

    // ---- build contiguous diff blocks (for "next / previous difference") ----
    const qint64 gap = qMax<qint64>(0, opt.mergeGap);
    qint64 blockStart = -1;
    qint64 lastDiff   = -1;

    for (int i = 0; i < n; ++i) {
        if (!isDiffState(r.leftStates.at(i)))
            continue;

        if (blockStart < 0) {
            blockStart = i;
        } else if (i - lastDiff - 1 > gap) {
            r.blocks.append(DiffBlock{ blockStart, lastDiff - blockStart + 1 });
            blockStart = i;
        }
        lastDiff = i;
    }
    if (blockStart >= 0)
        r.blocks.append(DiffBlock{ blockStart, lastDiff - blockStart + 1 });

    return r;
}