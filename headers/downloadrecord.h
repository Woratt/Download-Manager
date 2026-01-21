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
    QString m_filePath;
    QString m_status = "pending";
    QString m_expectedHash;
    QString m_actualHash;
    QString m_hashAlgorithm;
    QByteArray m_chunkHashes;
    bool m_isSSD;

    qint64 m_totalBytes = 0;
    qint64 m_downloadedBytes = 0;

    QDateTime m_createdAt;

    QVariantMap toVariantMap() const;
    static DownloadRecord fromVariantMap(const QVariantMap& data)
    {
        DownloadRecord record;
        return record;
    }
};

#endif // DOWNLOADRECORD_H
