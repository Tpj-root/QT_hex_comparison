#include "ColorSettingsWidget.h"

#include <QColorDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "ColorScheme.h"

namespace {

QString swatchStyle(const QColor &c)
{
    const double lum = 0.299 * c.redF() + 0.587 * c.greenF() + 0.114 * c.blueF();
    const QString fg = (lum > 0.5) ? QStringLiteral("#212121")
                                   : QStringLiteral("#FFFFFF");
    return QStringLiteral("QPushButton { background-color:%1; color:%2; "
                          "border:1px solid #555; padding:4px 8px; min-width:90px; }")
            .arg(c.name(), fg);
}

} // namespace

ColorSettingsWidget::ColorSettingsWidget(QWidget *parent) : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(10);

    // ---- Diff backgrounds ----
    auto *diffGroup = new QGroupBox(tr("Diff background colours"), this);
    auto *diffGrid  = new QGridLayout(diffGroup);
    diffGrid->setHorizontalSpacing(12);
    diffGrid->setVerticalSpacing(6);
    addRow(diffGrid, 0, tr("Identical bytes (equal)"),
           [] { return ColorScheme::instance().equal(); },
           [] (const QColor &c) { ColorScheme::instance().setEqual(c); });
    addRow(diffGrid, 1, tr("Different bytes (mismatch)"),
           [] { return ColorScheme::instance().different(); },
           [] (const QColor &c) { ColorScheme::instance().setDifferent(c); });
    addRow(diffGrid, 2, tr("Only one side has data"),
           [] { return ColorScheme::instance().onlyOneSide(); },
           [] (const QColor &c) { ColorScheme::instance().setOnlyOneSide(c); });
    addRow(diffGrid, 3, tr("Missing byte on this side"),
           [] { return ColorScheme::instance().missing(); },
           [] (const QColor &c) { ColorScheme::instance().setMissing(c); });
    addRow(diffGrid, 4, tr("Out of range / empty"),
           [] { return ColorScheme::instance().outOfRange(); },
           [] (const QColor &c) { ColorScheme::instance().setOutOfRange(c); });
    root->addWidget(diffGroup);

    // ---- Search highlights ----
    auto *searchGroup = new QGroupBox(tr("Search highlight colours"), this);
    auto *searchGrid  = new QGridLayout(searchGroup);
    searchGrid->setHorizontalSpacing(12);
    searchGrid->setVerticalSpacing(6);
    addRow(searchGrid, 0, tr("Search hit"),
           [] { return ColorScheme::instance().searchHit(); },
           [] (const QColor &c) { ColorScheme::instance().setSearchHit(c); });
    addRow(searchGrid, 1, tr("Current search hit"),
           [] { return ColorScheme::instance().currentHit(); },
           [] (const QColor &c) { ColorScheme::instance().setCurrentHit(c); });
    root->addWidget(searchGroup);

    // ---- Offset column ----
    auto *offsetGroup = new QGroupBox(tr("Offset column"), this);
    auto *offsetGrid  = new QGridLayout(offsetGroup);
    offsetGrid->setHorizontalSpacing(12);
    offsetGrid->setVerticalSpacing(6);
    addRow(offsetGrid, 0, tr("Offset column background"),
           [] { return ColorScheme::instance().offsetBg(); },
           [] (const QColor &c) { ColorScheme::instance().setOffsetBg(c); });
    addRow(offsetGrid, 1, tr("Offset column text"),
           [] { return ColorScheme::instance().offsetFg(); },
           [] (const QColor &c) { ColorScheme::instance().setOffsetFg(c); });
    root->addWidget(offsetGroup);

    // ---- Text ----
    auto *textGroup = new QGroupBox(tr("Hex / ASCII text"), this);
    auto *textGrid  = new QGridLayout(textGroup);
    textGrid->setHorizontalSpacing(12);
    textGrid->setVerticalSpacing(6);
    addRow(textGrid, 0, tr("Hex byte text"),
           [] { return ColorScheme::instance().text(); },
           [] (const QColor &c) { ColorScheme::instance().setText(c); });
    addRow(textGrid, 1, tr("ASCII character text"),
           [] { return ColorScheme::instance().asciiText(); },
           [] (const QColor &c) { ColorScheme::instance().setAsciiText(c); });
    root->addWidget(textGroup);

    // ---- View background ----
    auto *viewGroup = new QGroupBox(tr("View"), this);
    auto *viewGrid  = new QGridLayout(viewGroup);
    viewGrid->setHorizontalSpacing(12);
    viewGrid->setVerticalSpacing(6);
    addRow(viewGrid, 0, tr("View background (empty area)"),
           [] { return ColorScheme::instance().viewBackground(); },
           [] (const QColor &c) { ColorScheme::instance().setViewBackground(c); });
    root->addWidget(viewGroup);

    // ---- Bottom row ----
    auto *bottomRow = new QHBoxLayout;
    auto *resetBtn  = new QPushButton(tr("Reset to defaults"), this);
    bottomRow->addStretch(1);
    bottomRow->addWidget(resetBtn);
    root->addLayout(bottomRow);
    root->addStretch(1);

    connect(resetBtn, &QPushButton::clicked, this, &ColorSettingsWidget::onResetClicked);
    connect(&ColorScheme::instance(), &ColorScheme::changed,
            this, &ColorSettingsWidget::refreshAll);

    refreshAll();
}

void ColorSettingsWidget::addRow(QGridLayout *grid, int rowIndex,
                                 const QString &label,
                                 const Getter &getter,
                                 const Setter &setter)
{
    auto *labelWidget = new QLabel(label, this);
    auto *btn         = new QPushButton(this);
    btn->setCursor(Qt::PointingHandCursor);

    Row row;
    row.label  = label;
    row.button = btn;
    row.getter = getter;
    row.setter = setter;
    m_rows.append(row);

    // index for lambda capture (append already happened → last element)
    const int idx = m_rows.size() - 1;
    connect(btn, &QPushButton::clicked, this, [this, idx] {
        if (idx < 0 || idx >= m_rows.size()) return;
        Row &r = m_rows[idx];
        const QColor chosen = QColorDialog::getColor(
            r.getter(), this, r.label);
        if (chosen.isValid()) r.setter(chosen);
    });

    grid->addWidget(labelWidget, rowIndex, 0);
    grid->addWidget(btn,         rowIndex, 1, Qt::AlignLeft);
}

void ColorSettingsWidget::refreshAll()
{
    for (Row &r : m_rows) {
        if (!r.button) continue;
        const QColor c = r.getter();
        r.button->setStyleSheet(swatchStyle(c));
        r.button->setText(c.name(QColor::HexRgb).toUpper());
    }
}

void ColorSettingsWidget::onResetClicked()
{
    ColorScheme::instance().resetToDefaults();
}