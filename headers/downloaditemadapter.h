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
    QVector<DownloadRecord> toRecord(const QVector<QPair<DownloadItem*, std::shared_ptr<DownloadTask>>>& pairs);
};

#endif // DOWNLOADITEMADAPTER_H
