#include "BookmarkWidget.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "Bookmark.h"
#include "BookmarkManager.h"
#include "ByteFormatter.h"

namespace {

QString hexAddr(qint64 off, int digits = 8)
{
    return QStringLiteral("0x%1").arg(off, digits, 16, QChar('0')).toUpper();
}

QString colorStyleSheet(const QColor &color)
{
    const double lum = 0.299 * color.redF()
                     + 0.587 * color.greenF()
                     + 0.114 * color.blueF();
    const QString fg = (lum > 0.5) ? QStringLiteral("#212121") : QStringLiteral("#FFFFFF");
    return QStringLiteral("QPushButton { background-color: %1; color: %2; "
                          "border: 1px solid #555; padding: 4px; }")
            .arg(color.name(), fg);
}

} // namespace

BookmarkWidget::BookmarkWidget(BookmarkManager *manager, QWidget *parent)
    : QWidget(parent), m_manager(manager)
{
    m_currentColor = QColor(255, 193, 7);   // amber default

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(6);

    // -------- input group --------
    auto *inputGroup  = new QGroupBox(tr("Add bookmark"), this);
    auto *inputLayout = new QFormLayout(inputGroup);
    inputLayout->setLabelAlignment(Qt::AlignRight);

    m_startEdit = new QLineEdit(inputGroup);
    m_startEdit->setPlaceholderText(QStringLiteral("0x00000000"));
    m_startEdit->setMaximumWidth(180);
    inputLayout->addRow(tr("Start address:"), m_startEdit);

    m_endEdit = new QLineEdit(inputGroup);
    m_endEdit->setPlaceholderText(QStringLiteral("0x000000FF"));
    m_endEdit->setMaximumWidth(180);
    inputLayout->addRow(tr("End address (inclusive):"), m_endEdit);

    m_colorButton = new QPushButton(inputGroup);
    m_colorButton->setMinimumWidth(160);
    applyColorSwatch();
    inputLayout->addRow(tr("Color:"), m_colorButton);

    m_nameEdit = new QLineEdit(inputGroup);
    m_nameEdit->setPlaceholderText(tr("e.g. ELF header, copyright string, …"));
    inputLayout->addRow(tr("Name:"), m_nameEdit);

    auto *buttonRow = new QHBoxLayout;
    m_useSelectionButton = new QPushButton(tr("Use current hex selection"), inputGroup);
    m_addButton          = new QPushButton(tr("+ Add bookmark"), inputGroup);
    m_addButton->setDefault(true);
    buttonRow->addWidget(m_useSelectionButton);
    buttonRow->addWidget(m_addButton);
    buttonRow->addStretch(1);
    inputLayout->addRow(QString(), buttonRow);

    root->addWidget(inputGroup);

    // -------- list group --------
    auto *listGroup  = new QGroupBox(
        tr("Bookmarks   (newer entries override older on overlap)"), this);
    auto *listLayout = new QVBoxLayout(listGroup);
    listLayout->setContentsMargins(4, 4, 4, 4);

    m_table = new QTableWidget(listGroup);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels(
        { tr("Color"), tr("Name"), tr("Range"), tr("Length") });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_table->setColumnWidth(0, 56);
    m_table->setColumnWidth(2, 230);
    m_table->setColumnWidth(3, 110);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    listLayout->addWidget(m_table, 1);

    auto *bottomRow    = new QHBoxLayout;
    auto *removeButton = new QPushButton(tr("Remove selected"), listGroup);
    auto *clearButton  = new QPushButton(tr("Clear all"), listGroup);
    auto *saveButton   = new QPushButton(tr("Save to file…"), listGroup);
    auto *loadButton   = new QPushButton(tr("Load from file…"), listGroup);
    bottomRow->addWidget(removeButton);
    bottomRow->addWidget(clearButton);
    bottomRow->addStretch(1);
    bottomRow->addWidget(saveButton);
    bottomRow->addWidget(loadButton);
    listLayout->addLayout(bottomRow);

    root->addWidget(listGroup, 1);

    // -------- wiring --------
    connect(m_colorButton,        &QPushButton::clicked, this, &BookmarkWidget::onPickColor);
    connect(m_addButton,          &QPushButton::clicked, this, &BookmarkWidget::onAddClicked);
    connect(m_useSelectionButton, &QPushButton::clicked, this, &BookmarkWidget::onUseSelectionClicked);
    connect(removeButton,         &QPushButton::clicked, this, &BookmarkWidget::onRemoveClicked);
    connect(clearButton,          &QPushButton::clicked, this, &BookmarkWidget::onClearAllClicked);
    connect(saveButton,           &QPushButton::clicked, this, &BookmarkWidget::onSaveToFile);
    connect(loadButton,           &QPushButton::clicked, this, &BookmarkWidget::onLoadFromFile);

    connect(m_table, &QTableWidget::cellClicked,       this, &BookmarkWidget::onCellClicked);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &BookmarkWidget::onCellDoubleClicked);
    connect(m_table, &QWidget::customContextMenuRequested,
            this, &BookmarkWidget::onContextMenu);

    connect(m_manager, &BookmarkManager::changed,
            this, &BookmarkWidget::rebuildTable);

    rebuildTable();
}

void BookmarkWidget::applyColorSwatch()
{
    m_colorButton->setStyleSheet(colorStyleSheet(m_currentColor));
    m_colorButton->setText(m_currentColor.name(QColor::HexRgb).toUpper());
}

void BookmarkWidget::onPickColor()
{
    const QColor chosen = QColorDialog::getColor(
        m_currentColor, this, tr("Pick bookmark color"));
    if (chosen.isValid()) {
        m_currentColor = chosen;
        applyColorSwatch();
    }
}

void BookmarkWidget::setCurrentSelection(qint64 startOffset, qint64 endOffset)
{
    m_selStart = startOffset;
    m_selEnd   = endOffset;
}

void BookmarkWidget::onUseSelectionClicked()
{
    if (m_selStart < 0 || m_selEnd < 0) {
        QMessageBox::information(this, tr("No selection"),
            tr("Select bytes in the Hex Compare tab first, then click again."));
        return;
    }
    m_startEdit->setText(hexAddr(m_selStart));
    m_endEdit->setText(hexAddr(m_selEnd));
}

void BookmarkWidget::onAddClicked()
{
    qint64 start = 0, end = 0;
    if (!ByteFormatter::parseOffset(m_startEdit->text(), &start)) {
        QMessageBox::warning(this, tr("Invalid start"),
                             tr("Start address is not valid hex."));
        return;
    }
    if (!ByteFormatter::parseOffset(m_endEdit->text(), &end)) {
        QMessageBox::warning(this, tr("Invalid end"),
                             tr("End address is not valid hex."));
        return;
    }
    if (end < start) std::swap(start, end);

    const int id = m_manager->addBookmark(start, end + 1,  // UI inclusive → model exclusive
                                          m_currentColor, m_nameEdit->text());
    if (id < 0) {
        QMessageBox::warning(this, tr("Empty range"),
                             tr("Start and end are the same."));
        return;
    }
    emit bookmarksModified();
}

int BookmarkWidget::idAtRow(int row) const
{
    if (row < 0 || row >= m_table->rowCount()) return -1;
    const QTableWidgetItem *it = m_table->item(row, 0);
    return it ? it->data(Qt::UserRole).toInt() : -1;
}

void BookmarkWidget::onRemoveClicked()
{
    const int id = idAtRow(m_table->currentRow());
    if (id < 0) return;
    m_manager->removeBookmark(id);
    emit bookmarksModified();
}

void BookmarkWidget::onClearAllClicked()
{
    if (m_manager->count() == 0) return;
    if (QMessageBox::question(this, tr("Clear bookmarks"),
            tr("Remove all %1 bookmarks?").arg(m_manager->count())) == QMessageBox::Yes) {
        m_manager->clear();
        emit bookmarksModified();
    }
}

void BookmarkWidget::onSaveToFile()
{
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Save bookmarks"), QStringLiteral("bookmarks.json"),
        tr("Bookmark files (*.json *.bmk);;All files (*)"));
    if (path.isEmpty()) return;

    QString error;
    if (!m_manager->saveToFile(path, &error))
        QMessageBox::warning(this, tr("Save failed"), error);
    else
        QMessageBox::information(this, tr("Saved"),
            tr("Saved %1 bookmark(s) to %2").arg(m_manager->count()).arg(path));
}

void BookmarkWidget::onLoadFromFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Load bookmarks"), QString(),
        tr("Bookmark files (*.json *.bmk);;All files (*)"));
    if (path.isEmpty()) return;

    QString error;
    if (!m_manager->loadFromFile(path, &error))
        QMessageBox::warning(this, tr("Load failed"), error);
    else
        emit bookmarksModified();
}

void BookmarkWidget::onCellClicked(int row, int /*column*/)
{
    const int id = idAtRow(row);
    if (id < 0) return;
    if (const Bookmark *bm = m_manager->bookmarkById(id))
        emit navigateRequested(bm->start);
}

void BookmarkWidget::onCellDoubleClicked(int row, int /*column*/)
{
    onCellClicked(row, 0);
}

void BookmarkWidget::onContextMenu(const QPoint &pos)
{
    const QModelIndex idx = m_table->indexAt(pos);
    if (!idx.isValid()) return;

    const int id = idAtRow(idx.row());
    if (id < 0) return;
    const Bookmark *bm = m_manager->bookmarkById(id);
    if (!bm) return;

    QMenu menu(this);
    QAction *goAction     = menu.addAction(tr("Go to bookmark"));
    QAction *editAction   = menu.addAction(tr("Edit name / color…"));
    menu.addSeparator();
    QAction *removeAction = menu.addAction(tr("Remove"));

    QAction *chosen = menu.exec(m_table->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    if (chosen == goAction) {
        emit navigateRequested(bm->start);
    } else if (chosen == editAction) {
        bool ok = false;
        const QString newName = QInputDialog::getText(
            this, tr("Edit bookmark name"), tr("Name:"),
            QLineEdit::Normal, bm->name, &ok);
        if (!ok) return;
        const QColor newColor = QColorDialog::getColor(
            bm->color, this, tr("Pick bookmark color"));
        if (newColor.isValid()) {
            m_manager->updateBookmark(bm->id, bm->start, bm->end,
                                      newColor, newName);
            emit bookmarksModified();
        }
    } else if (chosen == removeAction) {
        m_manager->removeBookmark(bm->id);
        emit bookmarksModified();
    }
}

void BookmarkWidget::rebuildTable()
{
    const int row = m_table->currentRow();
    m_table->setRowCount(0);

    const auto &list = m_manager->bookmarks();
    m_table->setRowCount(list.size());

    for (int i = 0; i < list.size(); ++i) {
        const Bookmark &bm = list.at(i);

        auto *colorItem = new QTableWidgetItem;
        colorItem->setBackground(bm.color);
        colorItem->setData(Qt::UserRole, bm.id);
        m_table->setItem(i, 0, colorItem);

        m_table->setItem(i, 1, new QTableWidgetItem(bm.name));

        m_table->setItem(i, 2, new QTableWidgetItem(
            QStringLiteral("%1 – %2")
                .arg(hexAddr(bm.start), hexAddr(bm.end - 1))));

        m_table->setItem(i, 3, new QTableWidgetItem(
            QStringLiteral("%1 B").arg(bm.length())));
    }

    if (row >= 0 && row < m_table->rowCount())
        m_table->selectRow(row);
}