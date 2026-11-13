// FilePickerWidget.cpp
#include "FilePickerWidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

FilePickerWidget::FilePickerWidget(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(3);

    m_titleLabel = new QLabel(title, this);
    QFont tf = m_titleLabel->font();
    tf.setBold(true);
    m_titleLabel->setFont(tf);

    auto *row = new QHBoxLayout;
    row->setSpacing(4);

    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setReadOnly(true);
    m_pathEdit->setPlaceholderText(tr("No file selected…"));

    m_browseButton = new QPushButton(tr("Browse…"), this);
    m_clearButton  = new QPushButton(tr("✕"), this);
    m_clearButton->setFixedWidth(28);
    m_clearButton->setToolTip(tr("Unload this file"));

    m_sizeLabel = new QLabel(tr("—"), this);
    m_sizeLabel->setMinimumWidth(90);
    m_sizeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    row->addWidget(m_pathEdit, 1);
    row->addWidget(m_browseButton);
    row->addWidget(m_clearButton);
    row->addWidget(m_sizeLabel);

    root->addWidget(m_titleLabel);
    root->addLayout(row);

    connect(m_browseButton, &QPushButton::clicked, this, &FilePickerWidget::browse);
    connect(m_clearButton,  &QPushButton::clicked, this, [this] {
        m_pathEdit->clear();
        m_sizeLabel->setText(tr("—"));
        emit cleared();
    });
}

void FilePickerWidget::browse()
{
    const QString start = m_lastDir.isEmpty() ? QString() : m_lastDir;
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Select binary file"), start,
        tr("Binary files (*.bin *.hex *.dat *.rom *.img *.exe *.dll);;All files (*)"));

    if (path.isEmpty()) return;

    m_lastDir = QFileInfo(path).absolutePath();
    setFilePath(path);
    emit fileChosen(path);
}

void FilePickerWidget::setFilePath(const QString &path)
{
    m_pathEdit->setText(path);
    m_pathEdit->setToolTip(path);
}

QString FilePickerWidget::filePath() const { return m_pathEdit->text(); }

void FilePickerWidget::setSizeText(const QString &text) { m_sizeLabel->setText(text); }