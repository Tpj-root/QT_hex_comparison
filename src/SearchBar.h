// SearchBar.h
#pragma once
#include <QByteArray>
#include <QWidget>

class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;

class SearchBar : public QWidget
{
    Q_OBJECT
public:
    explicit SearchBar(QWidget *parent = nullptr);

    QByteArray pattern(bool *ok) const;

    void setHitInfo(int total, int currentIndex);

signals:
    void searchRequested();
    void nextRequested();
    void previousRequested();
    void cleared();

private:
    QLineEdit   *m_edit      = nullptr;
    QComboBox   *m_modeCombo = nullptr;
    QPushButton *m_findButton = nullptr;
    QPushButton *m_nextButton = nullptr;
    QPushButton *m_prevButton = nullptr;
    QLabel      *m_hitLabel  = nullptr;
};