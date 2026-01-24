#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QMap>

#include "downloaditem.h"
#include "threadpool.h"
#include "downloaddatabase.h"
#include "downloaditemadapter.h"

#include "downloadtypes.h"
#include "networkmanager.h"
#include "storagemanager.h"

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();
    void processDownloadRequest(const QString &url, const QString &saveDir, const DownloadTypes::UserChoice& userChoice);
    void setItemsFromDB();
    void prepareToExit();
private:
    ThreadPool *m_threadPool;
    DownloadDatabase *m_db;
    QVector<DownloadItem*> m_selectedItems;
    QVector<DownloadItem*> m_items;
    QVector<QString> m_urlsDownloading;
    QHash<DownloadItem*, DownloadTask*> m_itemTask;

    void createAndStartDownload(const QString &url, const QString &filePath, const QString& fileName, qint64 fileSize);
    DownloadTypes::ConflictResult checkForConflicts(const QString &url, const QString &filePuth);

    NetworkManager *m_networkManager;
    StorageManager *m_storageManager;
    QThread *m_storageThread;

    int numOfSavedTask{0};

    void checkPoolStatus();
    void saveAllAndQuit();
public slots:
    void changeBt(DownloadItem*, bool);
    void downloadAll();
    void pauseAll();
    void deleteAll();
    void deleteDownload(DownloadItem *item);
private slots:
    void finished();
signals:
    void showButtons();
    void hideButtons();
    void setDownloadItemFromDB(DownloadItem*);
    void downloadReadyToAdd(DownloadItem*);
    void conflictsDetected(const QString &url, const DownloadTypes::ConflictResult &result);
    void deleteDownloadItem(DownloadItem *item);
    void readyToQuit();
};

#endif // DOWNLOADMANAGER_H
