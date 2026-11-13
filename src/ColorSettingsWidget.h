#pragma once
#include <QColor>
#include <QList>
#include <QWidget>

#include <functional>

class QGridLayout;
class QPushButton;

/**
 * Live editor for every colour used by the hex view.
 * Two columns per row: label + colour swatch button.
 */
class ColorSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorSettingsWidget(QWidget *parent = nullptr);

private slots:
    void refreshAll();
    void onResetClicked();

private:
    using Getter = std::function<QColor()>;
    using Setter = std::function<void(const QColor &)>;

    struct Row {
        QString      label;
        QPushButton *button = nullptr;
        Getter       getter;
        Setter       setter;
    };

    void addRow(QGridLayout *grid, int rowIndex,
                const QString &label,
                const Getter &getter,
                const Setter &setter);

    QList<Row> m_rows;
};