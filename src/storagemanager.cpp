#include "../headers/storagemanager.h"

StorageManager::StorageManager(QObject *parent) : QObject(parent) {}

StorageManager::~StorageManager() {
    closeFile();
}

bool StorageManager::openFile(const QString &filePath, qint64 totalSize) {
    m_file.setFileName(filePath);
    m_totalSize = totalSize;

    if (!m_file.open(QIODevice::ReadWrite)) {
        emit errorOccurred("Не вдалося відкрити файл для запису: " + m_file.errorString());
        return false;
    }

    if (m_file.size() < totalSize) {
        if (!m_file.resize(totalSize)) {
            emit errorOccurred("Не вдалося виділити місце на диску!");
            return false;
        }
    }
    return true;
}

void StorageManager::writeChunk(int index, const QByteArray &data, const QByteArray &hash) {
    if (!m_file.isOpen()) return;

    if (m_chunkSize == 0) m_chunkSize = data.size();

    qint64 position = static_cast<qint64>(index) * m_chunkSize;

    if (!m_file.seek(position)) {
        emit errorOccurred("Помилка позиціювання у файлі!");
        return;
    }

    if (m_file.write(data) == -1) {
        emit errorOccurred("Помилка запису даних на диск!");
    } else {
        m_file.flush();
        emit chunkSaved(index);
    }

    if(data.size() < 1024 * 1024){
        emit savedLastChunk();
    }
}

void StorageManager::clearFile(){
    if (m_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_file.close();
    }
}

void StorageManager::closeFile() {
    if (m_file.isOpen()) {
        m_file.flush();
        m_file.close();
    }
}
