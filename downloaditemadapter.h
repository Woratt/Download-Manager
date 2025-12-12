#ifndef DOWNLOADITEMADAPTER_H
#define DOWNLOADITEMADAPTER_H

#include "downloadrecord.h"
#include "downloaditem.h"

class DownloadItemAdapter
{
public:
    static DownloadRecord toRecord(const DownloadItem& item){
        DownloadRecord record;

        // Основна інформація
        record.m_name = item.m_nameFileStr;
        record.m_url = item.m_url;
        record.m_directory = item.m_dir;

        // Прогрес
        record.m_totalBytes = item.m_bytesTotal;
        record.m_downloadedBytes = item.m_totalBytesReceived;
        //record.m_resumePosition = item.m_resumePosSize;

        // Статус та швидкість
        record.m_currentSpeed = item.m_currentSpeed;
        record.m_isChecked = item.m_isChecked;

        // Визначаємо статус автоматично
        if (item.m_percentages >= 100) {
            record.m_status = "completed";
        } else if (item.m_currentSpeed > 0) {
            record.m_status = "downloading";
        } else if (item.m_totalBytesReceived > 0) {
            record.m_status = "paused";
        } else {
            record.m_status = "pending";
        }

        // Таймстампи
        record.m_updatedAt = QDateTime::currentDateTime();
        if (record.m_createdAt.isNull()) {
            record.m_createdAt = record.m_updatedAt;
        }

        return record;
    }

    static void updateItemFromRecord(DownloadItem* item, const DownloadRecord& record){
        item->m_nameFileStr = record.m_name;
        item->m_url = record.m_url;
        item->m_dir = record.m_directory;
        item->m_bytesTotal = record.m_totalBytes;
        item->m_totalBytesReceived = record.m_downloadedBytes;
        //item->m_resumePosSize = record.m_resumePosition;
        item->m_currentSpeed = record.m_currentSpeed;
        item->m_isChecked = record.m_isChecked;
        //item->m_status = record.m_status;
        item->m_fromDB = true;

        // Відновлюємо відсотки
        if (record.m_totalBytes > 0) {
            item->m_percentages = (record.m_downloadedBytes * 100) / record.m_totalBytes;
        }
    }
};

#endif // DOWNLOADITEMADAPTER_H
