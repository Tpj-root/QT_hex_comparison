#include "BookmarkManager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <utility>

BookmarkManager::BookmarkManager(QObject *parent) : QObject(parent) {}

int BookmarkManager::addBookmark(qint64 start, qint64 end,
                                 const QColor &color, const QString &name)
{
    if (start > end) std::swap(start, end);
    if (start == end) return -1;

    Bookmark bm;
    bm.id    = m_nextId++;
    bm.start = start;
    bm.end   = end;
    bm.color = color.isValid() ? color : QColor(255, 193, 7);
    bm.name  = name.trimmed();
    if (bm.name.isEmpty())
        bm.name = QStringLiteral("Bookmark %1").arg(bm.id);

    m_bookmarks.append(bm);
    emit bookmarkAdded(bm.id);
    emit changed();
    return bm.id;
}

bool BookmarkManager::removeBookmark(int id)
{
    for (int i = 0; i < m_bookmarks.size(); ++i) {
        if (m_bookmarks[i].id == id) {
            m_bookmarks.remove(i);
            emit bookmarkRemoved(id);
            emit changed();
            return true;
        }
    }
    return false;
}

bool BookmarkManager::updateBookmark(int id, qint64 start, qint64 end,
                                     const QColor &color, const QString &name)
{
    if (start > end) std::swap(start, end);
    for (auto &bm : m_bookmarks) {
        if (bm.id == id) {
            bm.start = start;
            bm.end   = end;
            if (color.isValid())            bm.color = color;
            if (!name.trimmed().isEmpty())  bm.name  = name.trimmed();
            emit bookmarkUpdated(id);
            emit changed();
            return true;
        }
    }
    return false;
}

void BookmarkManager::clear()
{
    if (m_bookmarks.isEmpty()) return;
    m_bookmarks.clear();
    emit changed();
}

const Bookmark *BookmarkManager::bookmarkAt(qint64 offset) const
{
    // Bottom-up so the LAST-ADDED (bottom row) overrides on overlap.
    for (int i = m_bookmarks.size() - 1; i >= 0; --i)
        if (m_bookmarks[i].contains(offset))
            return &m_bookmarks[i];
    return nullptr;
}

const Bookmark *BookmarkManager::bookmarkById(int id) const
{
    for (const auto &bm : m_bookmarks)
        if (bm.id == id) return &bm;
    return nullptr;
}

bool BookmarkManager::saveToFile(const QString &path, QString *errorMessage) const
{
    QJsonArray arr;
    for (const auto &bm : m_bookmarks) {
        QJsonObject obj;
        obj[QStringLiteral("start")] = double(bm.start);
        obj[QStringLiteral("end")]   = double(bm.end);
        obj[QStringLiteral("color")] = bm.color.name(QColor::HexRgb);
        obj[QStringLiteral("name")]  = bm.name;
        arr.append(obj);
    }

    QJsonObject root;
    root[QStringLiteral("version")]   = 1;
    root[QStringLiteral("bookmarks")] = arr;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool BookmarkManager::loadFromFile(const QString &path, QString *errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) *errorMessage = parseError.errorString();
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonArray  arr  = root.value(QStringLiteral("bookmarks")).toArray();

    m_bookmarks.clear();
    m_nextId = 1;

    for (const auto &v : arr) {
        const QJsonObject obj = v.toObject();
        Bookmark bm;
        bm.id    = m_nextId++;
        bm.start = qint64(obj.value(QStringLiteral("start")).toDouble());
        bm.end   = qint64(obj.value(QStringLiteral("end")).toDouble());
        bm.color = QColor(obj.value(QStringLiteral("color")).toString());
        bm.name  = obj.value(QStringLiteral("name")).toString();
        if (bm.start < bm.end)
            m_bookmarks.append(bm);
    }

    emit changed();
    return true;
}