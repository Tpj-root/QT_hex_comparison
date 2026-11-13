// RangeControlWidget.h
#pragma once
#include <QWidget>

class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;

class RangeControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RangeControlWidget(QWidget *parent = nullptr);

    qint64 rangeStart()   const;
    qint64 rangeEnd()     const;   // exclusive, -1 = EOF
    int    bytesPerRow()  const;

signals:
    void rangeChanged(qint64 start, qint64 end, int bytesPerRow);

private slots:
    void emitChanged();

private:
    QLineEdit   *m_startEdit   = nullptr;
    QLineEdit   *m_endEdit     = nullptr;
    QComboBox   *m_bytesCombo  = nullptr;
    QPushButton *m_applyButton = nullptr;
    QLabel      *m_hintLabel   = nullptr;
};