// RangeControlWidget.cpp
#include "RangeControlWidget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "ByteFormatter.h"

RangeControlWidget::RangeControlWidget(QWidget *parent) : QWidget(parent)
{
    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(6);

    row->addWidget(new QLabel(tr("Start:"), this));
    m_startEdit = new QLineEdit(this);
    m_startEdit->setPlaceholderText(QStringLiteral("0x0"));
    m_startEdit->setMaximumWidth(120);
    m_startEdit->setToolTip(tr("First offset to compare — hex (e.g. 0x100 or 100). Empty = 0"));
    row->addWidget(m_startEdit);

    row->addWidget(new QLabel(tr("End:"), this));
    m_endEdit = new QLineEdit(this);
    m_endEdit->setPlaceholderText(tr("end of file"));
    m_endEdit->setMaximumWidth(120);
    m_endEdit->setToolTip(tr("Last offset to compare (inclusive) — hex. Empty = end of file"));
    row->addWidget(m_endEdit);

    row->addWidget(new QLabel(tr("Bytes / row:"), this));
    m_bytesCombo = new QComboBox(this);
    for (int n : { 8, 16, 24, 32, 48, 64 })
        m_bytesCombo->addItem(QString::number(n), n);
    m_bytesCombo->setCurrentIndex(1);          // 16
    row->addWidget(m_bytesCombo);

    m_applyButton = new QPushButton(tr("Apply"), this);
    row->addWidget(m_applyButton);

    m_hintLabel = new QLabel(tr("(range limits which bytes are compared & highlighted)"), this);
    m_hintLabel->setStyleSheet(QStringLiteral("color:#78909C;"));
    row->addWidget(m_hintLabel);
    row->addStretch(1);

    connect(m_applyButton, &QPushButton::clicked, this, &RangeControlWidget::emitChanged);
    connect(m_bytesCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RangeControlWidget::emitChanged);
}

qint64 RangeControlWidget::rangeStart() const
{
    qint64 v = 0;
    if (!ByteFormatter::parseOffset(m_startEdit->text(), &v) || v < 0) return 0;
    return v;
}

qint64 RangeControlWidget::rangeEnd() const
{
    const QString t = m_endEdit->text().trimmed();
    if (t.isEmpty()) return -1;                 // EOF

    qint64 v = 0;
    if (!ByteFormatter::parseOffset(t, &v) || v < 0) return -1;
    return v + 1;                               // UI is inclusive, engine is exclusive
}

int RangeControlWidget::bytesPerRow() const
{
    return m_bytesCombo->currentData().toInt();
}

void RangeControlWidget::emitChanged()
{
    emit rangeChanged(rangeStart(), rangeEnd(), bytesPerRow());
}