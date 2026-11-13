#pragma once
#include <QColor>
#include <QString>
#include <QStringList>
#include <QVector>

class BookmarkManager;

/**
 * Converts a set of search-hit offsets into individual bookmarks.
 *
 * Every hit becomes its own bookmark entry:
 *     start = hitOffset
 *     end   = hitOffset + patternLength
 *
 * The converter is stateless and lives entirely in this translation unit,
 * so it can be reused from any part of the app (or from a script later).
 */
class SearchBookmarkConverter
{
public:
    struct Options {
        QColor  color;                             // bookmark background
        QString namePrefix;                        // e.g. "DE AD BE EF"
        int     patternLength    = 1;              // bytes per hit
        bool    skipExisting     = true;           // skip bytes already bookmarked
        bool    perHitName       = true;           // append " #N" to each name
    };

    struct Result {
        int          created = 0;
        int          skipped = 0;
        QVector<int> newIds;                       // BookmarkManager ids
        QStringList  names;                        // bookmark names created
    };

    static Result convert(BookmarkManager *manager,
                          const QVector<qint64> &hitOffsets,
                          const Options &options);
};