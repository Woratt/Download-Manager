#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QObject>
#include <QFile>

class StorageManager : public QObject
{
    Q_OBJECT
public:
    StorageManager(QObject *parent = nullptr);
    ~StorageManager();

    bool openFile(const QString &filePath, qint64 totalSize);
    void clearFile();
    void closeFile();

public slots:
    void writeChunk(int index, const QByteArray &data, const QByteArray &hash);

signals:
    void chunkSaved(int index);
    void savedLastChunk();
    void errorOccurred(const QString &message);
private:
    QFile m_file;
    qint64 m_totalSize = 0;
    qint64 m_chunkSize = 0;
};

#endif // STORAGEMANAGER_H
