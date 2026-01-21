#include "../headers/storagemanager.h"

StorageManager::StorageManager(QObject *parent) : QObject(parent) {}

StorageManager::~StorageManager() {
    for(auto fileInfo = m_files.begin(); fileInfo != m_files.end(); ++fileInfo){
        closeFile(fileInfo.key());
    }
}

void StorageManager::openFile(const DownloadTypes::FileInfo &fileInfo) {
    if(!m_files[fileInfo]){
        QFile *file = new QFile(fileInfo.filePath, this);
        if (!file->open(QIODevice::ReadWrite)) {
            emit errorOccurred("Не вдалося відкрити файл для запису: " + file->errorString());
        }

        if (file->size() < fileInfo.fileSize) {
            if (!file->resize(fileInfo.fileSize)) {
                emit errorOccurred("Не вдалося виділити місце на диску!");
            }
        }
        m_files[fileInfo] = file;
    }else{
        if (!m_files[fileInfo]->open(QIODevice::ReadWrite)) {
            emit errorOccurred("Не вдалося відкрити файл для запису: " + m_files[fileInfo]->errorString());
        }
    }

    emit fileOpen(fileInfo);
}

void StorageManager::writeChunk(const DownloadTypes::FileInfo &fileInfo, int index, const QByteArray &data) {
    if (!m_files[fileInfo]->isOpen()) return;
    if(data.size() < m_chunkSize){
        m_data[fileInfo].push_back(data);
        writeToDisk(fileInfo, index);
    }else{
        m_data[fileInfo].push_back(data);
    }

    if(m_data[fileInfo].size() >= fileInfo.quantityOfChunks){
        writeToDisk(fileInfo, index);
    }
}

void StorageManager::writeToDisk(const DownloadTypes::FileInfo &fileInfo, qint64 lastIndex){
    if (!m_files.contains(fileInfo)) return;

    QFile *file = m_files[fileInfo];
    auto &chunks = m_data[fileInfo];
    if (chunks.isEmpty()) return;

    if (!chunks.isEmpty() && chunks.last().size() < m_chunkSize) {
        closeFile(fileInfo);
    }else{

        qint64 firstIndexInBatch = lastIndex - chunks.size() + 1;
        position = firstIndexInBatch * m_chunkSize;

        if (!file->seek(position)) {
            emit errorOccurred("Помилка позиціювання: " + file->errorString());
            return;
        }

        QElapsedTimer timer;
        timer.start();
        bool success = true;
        for (const QByteArray &chunk : chunks) {
            if (file->write(chunk) == -1) {
                success = false;
                break;
            }
        }


        if (!success) {
            emit errorOccurred("Помилка запису на диск!");
        } else {
            file->flush();
            emit chunkSaved(lastIndex);
        }
        qint64 writeTime = timer.elapsed();
        updateQuantityOfChunks(fileInfo, writeTime, chunks.size());

        chunks.clear();
    }
}

void StorageManager::flushAllData(const DownloadTypes::FileInfo &fileInfo){
    if (!m_files.contains(fileInfo)) return;

    QFile *file = m_files[fileInfo];
    auto &chunks = m_data[fileInfo];
    if (chunks.isEmpty()) return;

    bool success = true;
    for (const QByteArray &chunk : chunks) {
        if (file->write(chunk) == -1) {
            success = false;
            break;
        }
    }

    if (!success) {
        emit errorOccurred("Помилка запису на диск!");
    } else {
        file->flush();
    }

    if (!chunks.isEmpty() && chunks.last().size() < m_chunkSize) {
        emit savedLastChunk(fileInfo);
    }

    chunks.clear();

}

void StorageManager::updateQuantityOfChunks(const DownloadTypes::FileInfo &fileInfo, qint64 writeTime, qint64 dataSize){
    if (dataSize <= 0) return;

    qint64 avgTimePerChunk = writeTime / dataSize;

    if (avgTimePerChunk < 10) {
        if (fileInfo.quantityOfChunks > 8) emit changeQuantityOfChunks(fileInfo, -4);
    }
    else if (avgTimePerChunk < 30) {
        if (fileInfo.quantityOfChunks > 16) emit changeQuantityOfChunks(fileInfo, -2);
    }
    else if (avgTimePerChunk > 40 && avgTimePerChunk <= 100) {
        if (fileInfo.quantityOfChunks < 64) emit changeQuantityOfChunks(fileInfo, 4);
    }
    else if (avgTimePerChunk > 100) {
        if (fileInfo.quantityOfChunks < 128) emit changeQuantityOfChunks(fileInfo, 8);
    }

    if(fileInfo.quantityOfChunks > 120 || fileInfo.quantityOfChunks < 4){
        emit changeQuantityOfChunks(fileInfo, qBound(8LL, (qint64)fileInfo.quantityOfChunks, 128LL));
    }
}

void StorageManager::clearFile(const DownloadTypes::FileInfo &fileInfo){
    if (m_files[fileInfo]->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_files[fileInfo]->close();
    }
}

void StorageManager::closeFile(const DownloadTypes::FileInfo &fileInfo) {
    flushAllData(fileInfo);
    if (m_files[fileInfo]->isOpen()) {
        m_files[fileInfo]->flush();
        m_files[fileInfo]->close();
    }
}

void StorageManager::deleteAllInfo(const DownloadTypes::FileInfo &fileInfo){
    closeFile(fileInfo);
    m_files.remove(fileInfo);
    m_data.remove((fileInfo));
}
