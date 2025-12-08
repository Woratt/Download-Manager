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

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();
    void setItemsFromDB();
private:
    ThreadPool *m_threadPool;
    DownloadDatabase *m_db;
    QVector<DownloadItem*> m_selectedItems;
    QVector<DownloadItem*> m_items;
public slots:
    void startDownload(DownloadItem *item);
    void changeBt(DownloadItem*, bool);
    void downloadAll();
    void pauseAll();
    void deleteAll();
    void saveAll();
signals:
    void showButtons();
    void hideButtons();
    void setDownloadItemFromDB(DownloadItem*);
};

#endif // DOWNLOADMANAGER_H
