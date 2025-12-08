#include "downloadrecord.h"

DownloadRecord::DownloadRecord(QObject* parent) : QObject(parent) {}

QVariantMap DownloadRecord::toVariantMap() const{
    return {
        {"name", m_name},
        {"url", m_url},
        {"directory", m_directory},
        {"status", m_status},
        {"totalBytes", m_totalBytes},
        {"downloadedBytes", m_downloadedBytes},
        {"resumePosition", m_resumePosition},
        {"currentSpeed", m_currentSpeed},
        {"isChecked", m_isChecked},
        {"errorMessage", m_errorMessage},
        {"createdAt", m_createdAt.toString(Qt::ISODate)},
        {"updatedAt", m_updatedAt.toString(Qt::ISODate)}
    };
}

DownloadRecord::DownloadRecord(const DownloadRecord& record){
    m_name = record.m_name;
    m_url = record.m_url;
    m_directory = record.m_directory;
    m_status = record.m_status;

    m_totalBytes = record.m_totalBytes;
    m_downloadedBytes = record.m_downloadedBytes;
    m_resumePosition = record.m_resumePosition;

    m_currentSpeed = record.m_currentSpeed;
    m_createdAt = record.m_createdAt;
    m_updatedAt = record.m_updatedAt;

    m_isChecked = record.m_isChecked;
    m_errorMessage = record.m_errorMessage;
}

DownloadRecord& DownloadRecord::operator=(const DownloadRecord& record){
    m_name = record.m_name;
    m_url = record.m_url;
    m_directory = record.m_directory;
    m_status = record.m_status;

    m_totalBytes = record.m_totalBytes;
    m_downloadedBytes = record.m_downloadedBytes;
    m_resumePosition = record.m_resumePosition;

    m_currentSpeed = record.m_currentSpeed;
    m_createdAt = record.m_createdAt;
    m_updatedAt = record.m_updatedAt;

    m_isChecked = record.m_isChecked;
    m_errorMessage = record.m_errorMessage;

    return *this;
}
