#pragma once
#include <QColor>
#include <QWidget>

class BookmarkManager;
class QLineEdit;
class QPushButton;
class QTableWidget;

class BookmarkWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BookmarkWidget(BookmarkManager *manager, QWidget *parent = nullptr);

    void setCurrentSelection(qint64 startOffset, qint64 endOffset);

signals:
    void navigateRequested(qint64 offset);
    void bookmarksModified();

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onClearAllClicked();
    void onSaveToFile();
    void onLoadFromFile();
    void onUseSelectionClicked();
    void onCellClicked(int row, int column);
    void onCellDoubleClicked(int row, int column);
    void onContextMenu(const QPoint &pos);
    void onPickColor();
    void rebuildTable();

private:
    int  idAtRow(int row) const;
    void applyColorSwatch();

    BookmarkManager *m_manager = nullptr;
    QLineEdit       *m_startEdit = nullptr;
    QLineEdit       *m_endEdit   = nullptr;
    QLineEdit       *m_nameEdit  = nullptr;
    QPushButton     *m_colorButton = nullptr;
    QPushButton     *m_addButton = nullptr;
    QPushButton     *m_useSelectionButton = nullptr;
    QTableWidget    *m_table = nullptr;
    QColor           m_currentColor;

    qint64 m_selStart = -1;
    qint64 m_selEnd   = -1;
};