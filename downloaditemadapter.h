#ifndef DOWNLOADITEMADAPTER_H
#define DOWNLOADITEMADAPTER_H

#include "downloadrecord.h"
#include "downloaditem.h"
#include "downloadtask.h"

class DownloadAdapter
{
public:
    static DownloadRecord toRecord(const QPair<DownloadItem*, DownloadTask*>& pair){
        DownloadRecord record;


        record.m_name = pair.first->m_nameFileStr;
        record.m_filePath = pair.first->m_filePath;
        record.m_url = pair.first->getUrl();
        record.m_downloadedBytes = pair.first->m_totalBytesReceived;
        record.m_totalBytes = pair.first->m_bytesTotal;

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

        return record;
    }

    static void updateItemFromRecord(DownloadItem* item, const DownloadRecord& record){
        item->m_nameFileStr = record.m_name;
        item->m_filePath = record.m_filePath;
        item->m_url = record.m_url;
        item->m_bytesTotal = record.m_downloadedBytes;
        item->m_bytesTotal = record.m_totalBytes;
        QString status = record.m_status;
        if (record.m_status == "pending") item->chackWhatStatus(DownloadTask::Pending);
        if (record.m_status == "downloading") item->chackWhatStatus(DownloadTask::Downloading);
        if (record.m_status == "resumed") item->chackWhatStatus(DownloadTask::Resumed);
        if (record.m_status == "start_new_task") item->chackWhatStatus(DownloadTask::StartNewTask);
        if (record.m_status == "resumed_in_pending") item->chackWhatStatus(DownloadTask::ResumedInPending);
        if (record.m_status == "resumed_in_downloading") item->chackWhatStatus(DownloadTask::ResumedInDownloading);
        if (record.m_status == "paused") item->chackWhatStatus(DownloadTask::Paused);
        if (record.m_status == "paused_new") item->chackWhatStatus(DownloadTask::PausedNew);
        if (record.m_status == "paused_resume") item->chackWhatStatus(DownloadTask::PausedResume);
        if (record.m_status == "completed") item->chackWhatStatus(DownloadTask::Completed);
        if (record.m_status == "error") item->chackWhatStatus(DownloadTask::Error);
        if (record.m_status == "cancelled") item->chackWhatStatus(DownloadTask::Cancelled);
        if (record.m_status == "deleted") item->chackWhatStatus(DownloadTask::Deleted);
        item->onProgressChanged(record.m_downloadedBytes, record.m_totalBytes);
    }

    static void updateTaskFromRecord(DownloadTask* task, const DownloadRecord& record){
        task->m_totalBytesWritten = record.m_downloadedBytes;
        QString status = record.m_status;
        if (record.m_status == "pending") task->m_status = DownloadTask::Pending;
        if (record.m_status == "downloading") task->m_status = DownloadTask::Downloading;
        if (record.m_status == "resumed") task->m_status = DownloadTask::Resumed;
        if (record.m_status == "start_new_task") task->m_status = DownloadTask::StartNewTask;
        if (record.m_status == "resumed_in_pending") task->m_status = DownloadTask::ResumedInPending;
        if (record.m_status == "resumed_in_downloading") task->m_status = DownloadTask::ResumedInDownloading;
        if (record.m_status == "paused") task->m_status = DownloadTask::Paused;
        if (record.m_status == "paused_new") task->m_status = DownloadTask::PausedNew;
        if (record.m_status == "paused_resume") task->m_status = DownloadTask::PausedResume;
        if (record.m_status == "completed") task->m_status = DownloadTask::Completed;
        if (record.m_status == "error") task->m_status = DownloadTask::Error;
        if (record.m_status == "cancelled") task->m_status = DownloadTask::Cancelled;
        if (record.m_status == "deleted") task->m_status = DownloadTask::Deleted;
    }
};

#endif // DOWNLOADITEMADAPTER_H
