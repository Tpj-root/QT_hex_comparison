#pragma once
#include <QColor>
#include <QObject>
#include <QVector>

#include "Bookmark.h"

class BookmarkManager : public QObject
{
    Q_OBJECT
public:
    explicit BookmarkManager(QObject *parent = nullptr);

    const QVector<Bookmark> &bookmarks() const { return m_bookmarks; }
    int count() const { return m_bookmarks.size(); }

    int  addBookmark(qint64 start, qint64 end,
                     const QColor &color, const QString &name);
    bool removeBookmark(int id);
    bool updateBookmark(int id, qint64 start, qint64 end,
                        const QColor &color, const QString &name);
    void clear();

    // Overlap rule: LAST-ADDED (bottom of the list) wins.
    const Bookmark *bookmarkAt(qint64 offset) const;
    const Bookmark *bookmarkById(int id) const;

    bool saveToFile(const QString &path, QString *errorMessage = nullptr) const;
    bool loadFromFile(const QString &path, QString *errorMessage = nullptr);

signals:
    void changed();
    void bookmarkAdded(int id);
    void bookmarkRemoved(int id);
    void bookmarkUpdated(int id);

private:
    QVector<Bookmark> m_bookmarks;
    int m_nextId = 1;
};