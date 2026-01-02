#ifndef DOWNLOADITEMADAPTER_H
#define DOWNLOADITEMADAPTER_H

#include "downloadrecord.h"
#include "downloaditem.h"
#include "downloadtask.h"

class DownloadAdapter
{
public:
    static QVector<DownloadRecord> toRecord(const QVector<QPair<DownloadItem*, DownloadTask*>>& pairs){
        QVector<DownloadRecord> records;
        for(auto pair : pairs){
            DownloadRecord record;

            record.m_name = pair.first->m_nameFileStr;
            record.m_filePath = pair.first->m_filePath;
            record.m_url = pair.first->getUrl();
            record.m_downloadedBytes = pair.second->m_resumeDownloadPos;
            record.m_totalBytes = pair.first->m_bytesTotal;
            record.m_expectedHash = pair.second->m_remoteExpectedHash;
            record.m_actualHash = pair.second->m_actualHash;
            if(pair.second->m_activeAlgorithm == QCryptographicHash::Sha256){
                record.m_hashAlgorithm = "Sha256";
            }else{
                record.m_hashAlgorithm = "md5";
            }

            QByteArray serializedChunks;
            QDataStream out(&serializedChunks, QIODevice::WriteOnly);
            out << pair.second->m_chunkHashes;

            record.m_chunkHashes = serializedChunks;

            DownloadTask::Status status = pair.second->getStatus();
            switch(status) {
            case DownloadTask::Status::Pending: record.m_status = "pending"; break;
            case DownloadTask::Status::Downloading: record.m_status = "downloading"; break;
            case DownloadTask::Status::Resumed: record.m_status = "resumed"; break;
            case DownloadTask::Status::StartNewTask: record.m_status = "start_new_task"; break;
            case DownloadTask::Status::ResumedInPending: record.m_status = "resumed_in_pending"; break;
            case DownloadTask::Status::ResumedInDownloading: record.m_status = "resumed_in_downloading"; break;
            case DownloadTask::Status::Paused: record.m_status = "paused"; break;
            case DownloadTask::Status::PausedNew: record.m_status = "paused_new"; break;
            case DownloadTask::Status::PausedResume: record.m_status = "paused_resume"; break;
            case DownloadTask::Status::Completed: record.m_status = "completed"; break;
            case DownloadTask::Status::Error: record.m_status = "error"; break;
            case DownloadTask::Status::Cancelled: record.m_status = "cancelled"; break;
            case DownloadTask::Status::Deleted: record.m_status = "deleted"; break;
            }
            records.append(record);
        }

        return records;
    }
};

#endif // DOWNLOADITEMADAPTER_H
