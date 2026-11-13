// SearchEngine.h
#pragma once
#include <QByteArray>
#include <QVector>

class SearchEngine
{
public:
    struct Hit { qint64 offset = 0; int length = 0; };

    struct Hits {
        QVector<Hit> left;
        QVector<Hit> right;
    };

    static QVector<Hit> findAll(const QByteArray &haystack,
                                const QByteArray &needle,
                                int maxHits = 100000);

    static Hits searchBoth(const QByteArray &left,
                           const QByteArray &right,
                           const QByteArray &needle,
                           int maxHits = 100000);
};