#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QObject>
#include <QFile>
#include <QMap>
#include <QVector>
#include <QElapsedTimer>

#include "downloadtypes.h"


class StorageManager : public QObject
{
    Q_OBJECT
public:
    StorageManager(QObject *parent = nullptr);
    ~StorageManager();
public slots:
    void openFile(const DownloadTypes::DownloadRecord &fileInfo);
    void writeChunk(const DownloadTypes::DownloadRecord &fileInfo, int index, const QByteArray &data);
    void clearFile(const DownloadTypes::DownloadRecord &fileInfo);
    void closeFile(const DownloadTypes::DownloadRecord &ileInfo);
    void deleteAllInfo(const DownloadTypes::DownloadRecord &fileInfo);
signals:
    void chunkSaved(int index);
    void savedLastChunk(const DownloadTypes::DownloadRecord &fileInfo);
    void errorOccurred(const QString &message);
    void fileOpen(const DownloadTypes::DownloadRecord &fileInfo);
    void changeQuantityOfChunks(const DownloadTypes::DownloadRecord &fileInfo, qint64 valueOfChange);
private:
    qint64 m_chunkSize{1024 * 1024};
    qint64 position{0};

    QMap<DownloadTypes::DownloadRecord, QVector<QByteArray>> m_data;

    QMap<DownloadTypes::DownloadRecord, std::shared_ptr<QFile>> m_files;

    void writeToDisk(const DownloadTypes::DownloadRecord &fileInfo, qint64 index);
    void flushAllData(const DownloadTypes::DownloadRecord &fileInfo);

    void updateQuantityOfChunks(const DownloadTypes::DownloadRecord &fileInfo, qint64 writeTime, qint64 dataSize);
};

#endif // STORAGEMANAGER_H
