#ifndef DOWNLOADRECORD_H
#define DOWNLOADRECORD_H

#include <QObject>
#include <QVariantMap>
#include <QDateTime>

class DownloadRecord : public QObject
{
    Q_OBJECT
public:
    DownloadRecord(QObject* parent = nullptr);
    DownloadRecord(const DownloadRecord&);

    DownloadRecord& operator=(const DownloadRecord&);

    QString m_name;
    QString m_url;
    QString m_directory;
    QString m_status = "pending";

    qint64 m_totalBytes = 0;
    qint64 m_downloadedBytes = 0;
    qint64 m_resumePosition = 0;

    qint64 m_currentSpeed = 0;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;

    bool m_isChecked = false;
    QString m_errorMessage;

    QVariantMap toVariantMap() const;
    static DownloadRecord fromVariantMap(const QVariantMap& data)
    {
        DownloadRecord record;
        record.m_name = data["name"].toString();
        record.m_url = data["url"].toString();
        record.m_directory = data["directory"].toString();
        record.m_status = data["status"].toString();
        record.m_totalBytes = data["totalBytes"].toLongLong();
        record.m_downloadedBytes = data["downloadedBytes"].toLongLong();
        record.m_resumePosition = data["resumePosition"].toLongLong();
        record.m_currentSpeed = data["currentSpeed"].toLongLong();
        record.m_isChecked = data["isChecked"].toBool();
        record.m_errorMessage = data["errorMessage"].toString();
        record.m_createdAt = QDateTime::fromString(data["createdAt"].toString(), Qt::ISODate);
        record.m_updatedAt = QDateTime::fromString(data["updatedAt"].toString(), Qt::ISODate);
        return record;
    }
};

#endif // DOWNLOADRECORD_H
