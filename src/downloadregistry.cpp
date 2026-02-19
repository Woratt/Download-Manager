#include "../headers/downloadregistry.h"

DownloadRegistry::DownloadRegistry(QObject *parent): QObject(parent) {}

QUuid DownloadRegistry::addRecord(DownloadTypes::DownloadRecord&& record){
    QMutexLocker locker(&m_mutex);
    QUuid id = QUuid::createUuid();
    m_records[id] = std::move(record);
    return id;
}

void DownloadRegistry::updateProgress(QUuid id, qint64 received, qint64 total){
    QMutexLocker locker(&m_mutex);
    if (m_records.contains(id)) {
        auto &record = m_records[id];
        record.downloadedBytes = received;
        record.totalBytes = total;
        emit progressChanged(id, received, total);
    }
}

void DownloadRegistry::updateStatus(QUuid id, DownloadTypes::DownloadStatus status){
    QMutexLocker locker(&m_mutex);
    if(m_records.contains(id)){
        auto &record = m_records[id];
        record.status = status;
        emit statusChanged(id, status);
    }
}

DownloadTypes::DownloadRecord DownloadRegistry::getRecord(QUuid id) const{
    QMutexLocker locker(&m_mutex);
    return m_records.value(id);
}

QList<DownloadTypes::DownloadRecord> DownloadRegistry::getAllRecords() const {
    QMutexLocker locker(&m_mutex);
    return m_records.values();
}
