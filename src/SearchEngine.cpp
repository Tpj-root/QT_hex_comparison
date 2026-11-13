// SearchEngine.cpp
#include "SearchEngine.h"

QVector<SearchEngine::Hit> SearchEngine::findAll(const QByteArray &haystack,
                                                 const QByteArray &needle,
                                                 int maxHits)
{
    QVector<Hit> hits;
    if (needle.isEmpty() || haystack.isEmpty()) return hits;

    qint64 pos = 0;
    while (hits.size() < maxHits) {
        const qint64 found = haystack.indexOf(needle, int(pos));
        if (found < 0) break;
        hits.append(Hit{ found, int(needle.size()) });
        pos = found + 1;                 // overlapping matches are reported too
    }
    return hits;
}

SearchEngine::Hits SearchEngine::searchBoth(const QByteArray &left,
                                            const QByteArray &right,
                                            const QByteArray &needle,
                                            int maxHits)
{
    Hits h;
    h.left  = findAll(left,  needle, maxHits);
    h.right = findAll(right, needle, maxHits);
    return h;
}