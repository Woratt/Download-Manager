#include "../headers/downloadrecord.h"

DownloadRecord::DownloadRecord(QObject* parent) : QObject(parent) {}

QVariantMap DownloadRecord::toVariantMap() const{
    return {
        /*{"name", m_name},
        {"url", m_url},
        {"directory", m_directory},
        {"status", m_status},
        {"totalBytes", m_totalBytes},
        {"downloadedBytes", m_downloadedBytes},
        {"resumePosition", m_resumePosition},
        {"createdAt", m_createdAt.toString(Qt::ISODate)},*/
    };
}

DownloadRecord::DownloadRecord(const DownloadRecord& record){
    m_name = record.m_name;
    m_url = record.m_url;
    m_status = record.m_status;
    m_filePath = record.m_filePath;

    m_totalBytes = record.m_totalBytes;
    m_downloadedBytes = record.m_downloadedBytes;

    m_expectedHash = record.m_expectedHash;
    m_actualHash = record.m_actualHash;
    m_hashAlgorithm = record.m_hashAlgorithm;
    m_chunkHashes = record.m_chunkHashes;

    m_createdAt = record.m_createdAt;

}

DownloadRecord& DownloadRecord::operator=(const DownloadRecord& record){
    m_name = record.m_name;
    m_url = record.m_url;
    m_filePath = record.m_filePath;
    m_status = record.m_status;

    m_totalBytes = record.m_totalBytes;
    m_downloadedBytes = record.m_downloadedBytes;
    //m_resumePosition = record.m_resumePosition;

    m_createdAt = record.m_createdAt;

    return *this;
}
