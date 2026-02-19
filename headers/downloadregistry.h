#ifndef DOWNLOADREGISTRY_H
#define DOWNLOADREGISTRY_H

#include <QObject>
#include <QUuid>
#include <QHash>
#include <QMutex>

#include "downloadtypes.h"

class DownloadRegistry : public QObject
{
    Q_OBJECT
public:
    explicit DownloadRegistry(QObject *parent = nullptr);
    QUuid addRecord(DownloadTypes::DownloadRecord&& record);
    DownloadTypes::DownloadRecord getRecord(QUuid id) const;
    QList<DownloadTypes::DownloadRecord> getAllRecords() const;
public slots:
    void updateProgress(QUuid id, qint64 received, qint64 total);
    void updateStatus(QUuid id, DownloadTypes::DownloadStatus status);
signals:
    void progressChanged(QUuid id, qint64 received, qint64 total);
    void statusChanged(QUuid id, DownloadTypes::DownloadStatus status);
private:
    mutable QMutex m_mutex;
    QHash<QUuid, DownloadTypes::DownloadRecord> m_records;
};

#endif // DOWNLOADREGISTRY_H
