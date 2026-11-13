// BinaryFile.h
#pragma once
#include <QByteArray>
#include <QObject>
#include <QString>

class BinaryFile : public QObject
{
    Q_OBJECT
public:
    explicit BinaryFile(QObject *parent = nullptr);

    bool load(const QString &filePath, QString *errorMessage = nullptr);
    void clear();

    bool             isLoaded() const { return m_loaded; }
    const QByteArray &data()    const { return m_data; }
    qint64           size()     const { return m_data.size(); }
    const QString   &filePath() const { return m_filePath; }
    QString          fileName() const;

signals:
    void changed();

private:
    QByteArray m_data;
    QString    m_filePath;
    bool       m_loaded = false;
};