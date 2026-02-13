#ifndef DOWNLOADITEMADAPTER_H
#define DOWNLOADITEMADAPTER_H

#include <QVector>

#include "downloadrecord.h"
#include "downloadtask.h"

class DownloadAdapter
{
public:
    DownloadAdapter(){};
    ~DownloadAdapter(){};
    QVector<DownloadRecord> toRecords(const QVector<QPair<DownloadItem*, std::shared_ptr<DownloadTask>>>& pairs);
    DownloadRecord toRecord(const DownloadItem* item, std::shared_ptr<DownloadTask> task);
};

#endif // DOWNLOADITEMADAPTER_H
