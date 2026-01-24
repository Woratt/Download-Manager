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
    void openFile(const DownloadTypes::FileInfo &fileInfo);
    void writeChunk(const DownloadTypes::FileInfo &fileInfo, int index, const QByteArray &data);
    void clearFile(const DownloadTypes::FileInfo &fileInfo);
    void closeFile(const DownloadTypes::FileInfo &ileInfo);
    void deleteAllInfo(const DownloadTypes::FileInfo &fileInfo);
signals:
    void chunkSaved(int index);
    void savedLastChunk(const DownloadTypes::FileInfo &fileInfo);
    void errorOccurred(const QString &message);
    void fileOpen(const DownloadTypes::FileInfo &fileInfo);
    void changeQuantityOfChunks(const DownloadTypes::FileInfo &fileInfo, qint64 valueOfChange);
private:
    qint64 m_chunkSize{1024 * 1024};
    qint64 position{0};

    QMap<DownloadTypes::FileInfo, QVector<QByteArray>> m_data;

    QMap<DownloadTypes::FileInfo, QFile*> m_files;

    void writeToDisk(const DownloadTypes::FileInfo &fileInfo, qint64 index);
    void flushAllData(const DownloadTypes::FileInfo &fileInfo);

    void updateQuantityOfChunks(const DownloadTypes::FileInfo &fileInfo, qint64 writeTime, qint64 dataSize);
};

#endif // STORAGEMANAGER_H
