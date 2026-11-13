#include "SearchBookmarkConverter.h"

#include "Bookmark.h"
#include "BookmarkManager.h"

SearchBookmarkConverter::Result SearchBookmarkConverter::convert(
    BookmarkManager *manager,
    const QVector<qint64> &hitOffsets,
    const Options &opt)
{
    Result r;
    if (!manager || hitOffsets.isEmpty()) return r;
    if (opt.patternLength <= 0)           return r;

    QVector<Bookmark> toAdd;
    toAdd.reserve(hitOffsets.size());

    const QString baseName = opt.namePrefix.trimmed().isEmpty()
                                 ? QStringLiteral("Search hit")
                                 : opt.namePrefix.trimmed();

    int hitIndex = 0;
    for (qint64 off : hitOffsets) {
        if (off < 0) continue;
        ++hitIndex;

        // ---- skip duplicates (a byte already covered by *any* bookmark) ----
        if (opt.skipExisting && manager->bookmarkAt(off)) {
            ++r.skipped;
            continue;
        }

        Bookmark bm;
        bm.start = off;
        bm.end   = off + opt.patternLength;
        bm.color = opt.color.isValid() ? opt.color : QColor(255, 193, 7);
        bm.name  = opt.perHitName
                       ? QStringLiteral("%1 #%2").arg(baseName).arg(hitIndex)
                       : baseName;

        toAdd.append(bm);
        r.names.append(bm.name);
    }

    if (toAdd.isEmpty()) return r;

    r.newIds  = manager->addBookmarksBatch(toAdd);
    r.created = toAdd.size();
    return r;
}