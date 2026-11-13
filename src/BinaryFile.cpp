// BinaryFile.cpp
#include "BinaryFile.h"
#include <QFileInfo>
#include <QFile>

BinaryFile::BinaryFile(QObject *parent) : QObject(parent) {}

bool BinaryFile::load(const QString &filePath, QString *errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }
    m_data   = file.readAll();
    m_filePath = QFileInfo(filePath).absoluteFilePath();
    m_loaded = true;
    emit changed();
    return true;
}

void BinaryFile::clear()
{
    m_data.clear();
    m_filePath.clear();
    m_loaded = false;
    emit changed();
}

QString BinaryFile::fileName() const
{
    return m_filePath.isEmpty() ? QString() : QFileInfo(m_filePath).fileName();
}