// FilePickerWidget.h
#pragma once
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

class FilePickerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FilePickerWidget(const QString &title, QWidget *parent = nullptr);

    void    setFilePath(const QString &path);
    QString filePath() const;
    void    setSizeText(const QString &text);

signals:
    void fileChosen(const QString &path);
    void cleared();

private slots:
    void browse();

private:
    QLabel      *m_titleLabel  = nullptr;
    QLineEdit   *m_pathEdit    = nullptr;
    QPushButton *m_browseButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QLabel      *m_sizeLabel   = nullptr;
    QString      m_lastDir;
};