#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <Qfile>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QMap>

#include "downloaditem.h"
#include "threadpool.h"
#include "downloaddatabase.h"
#include "downloaditemadapter.h"

#include "downloadtypes.h"

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();
    void processDownloadRequest(const QString &url, const QString &saveDir, const DownloadTypes::UserChoice& userChoice);
    void setItemsFromDB();
private:
    ThreadPool *m_threadPool;
    DownloadDatabase *m_db;
    QVector<DownloadItem*> m_selectedItems;
    QVector<DownloadItem*> m_items;
    QVector<QString> m_urlsDownloading;
    QHash<DownloadItem*, DownloadTask*> m_itemTask;

    QString createDownloadPath(const QString &url, const QString &saveDir, const QString &suggestedName = "");
    void createAndStartDownload(const QString &url, const QString &filePath, const QString& fileName = "");
    DownloadTypes::ConflictResult checkForConflicts(const QString &url, const QString &saveDir, const QString &suggestedName = "");
public slots:
    void startDownload(DownloadItem *item);
    void changeBt(DownloadItem*, bool);
    void downloadAll();
    void pauseAll();
    void deleteAll();
    void saveAll();
private slots:
    void finished();
signals:
    void showButtons();
    void hideButtons();
    void setDownloadItemFromDB(DownloadItem*);
    void downloadReadyToAdd(DownloadItem*);
    void conflictsDetected(const QString &url, const DownloadTypes::ConflictResult &result);
};

#endif // DOWNLOADMANAGER_H
