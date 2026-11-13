// SearchBar.cpp
#include "SearchBar.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QStringList>

#include "ByteFormatter.h"

SearchBar::SearchBar(QWidget *parent) : QWidget(parent)
{
    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(6);

    row->addWidget(new QLabel(tr("Find bytes:"), this));

    m_edit = new QLineEdit(this);
    m_edit->setPlaceholderText(tr("e.g.  50 4B 03 04   or   \\x50\\x4B   or   ASCII text"));
    m_edit->setToolTip(tr("Hex bytes separated by spaces (DE AD BE EF) "
                          "or literal ASCII text."));
    row->addWidget(m_edit, 1);

    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem(tr("Auto"));
    m_modeCombo->addItem(tr("Hex"));
    m_modeCombo->addItem(tr("ASCII"));
    row->addWidget(m_modeCombo);

    m_findButton = new QPushButton(tr("Find all"), this);
    m_prevButton = new QPushButton(tr("◀"), this);
    m_nextButton = new QPushButton(tr("▶"), this);
    m_prevButton->setFixedWidth(30);
    m_nextButton->setFixedWidth(30);
    row->addWidget(m_findButton);
    row->addWidget(m_prevButton);
    row->addWidget(m_nextButton);

    m_hitLabel = new QLabel(tr("no search"), this);
    m_hitLabel->setMinimumWidth(130);
    row->addWidget(m_hitLabel);

    connect(m_findButton, &QPushButton::clicked, this, &SearchBar::searchRequested);
    connect(m_nextButton, &QPushButton::clicked, this, &SearchBar::nextRequested);
    connect(m_prevButton, &QPushButton::clicked, this, &SearchBar::previousRequested);
    connect(m_edit, &QLineEdit::returnPressed, this, &SearchBar::searchRequested);
}

QByteArray SearchBar::pattern(bool *ok) const
{
    if (ok) *ok = false;

    const QString text = m_edit->text().trimmed();
    if (text.isEmpty()) return {};

    const int mode = m_modeCombo->currentIndex();

    if (mode == 2) {                                   // ASCII
        const QByteArray b = text.toLatin1();
        if (ok) *ok = !b.isEmpty();
        return b;
    }

    if (mode == 1) {                                   // strict Hex
        QString t = text;
        t.replace(QLatin1String("\\x"), QLatin1String(" "));
        t.replace(QLatin1String("0x"),  QLatin1String(" "));
        t.replace(QLatin1Char(','), QLatin1Char(' '));

        const QStringList parts = t.split(QRegularExpression(QStringLiteral("\\s+")),
                                          Qt::SkipEmptyParts);
        QByteArray out;
        for (const QString &p : parts) {
            bool good = false;
            const uint v = p.toUInt(&good, 16);
            if (!good || v > 0xFF || p.size() > 2) return {};
            out.append(char(v));
        }
        if (ok) *ok = !out.isEmpty();
        return out;
    }

    return ByteFormatter::parseBytePattern(text, ok);  // Auto
}

void SearchBar::setHitInfo(int total, int currentIndex)
{
    if (total <= 0) {
        m_hitLabel->setText(tr("no match"));
        return;
    }
    m_hitLabel->setText(tr("%1 hit(s) — %2/%3")
                            .arg(total).arg(currentIndex + 1).arg(total));
}