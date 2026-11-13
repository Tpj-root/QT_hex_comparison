#pragma once
#include <QColor>
#include <QString>

struct Bookmark {
    int     id    = 0;
    qint64  start = 0;
    qint64  end   = 0;      // exclusive
    QColor  color;
    QString name;

    bool   contains(qint64 offset) const { return offset >= start && offset < end; }
    qint64 length() const { return end - start; }
};