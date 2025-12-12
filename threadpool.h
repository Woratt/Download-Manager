#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QObject>
#include <QHash>
#include <QPair>
#include <QThread>
#include <QQueue>

#include "downloaditem.h"
#include "downloadtask.h"

class ThreadPool : public QObject
{
    Q_OBJECT
public:
    explicit ThreadPool(QObject *parent = nullptr);
    void addTask(DownloadTask*);
    ~ThreadPool();
public slots:
    void onTaskFinished(DownloadTask*);
    //void onDeleteRequested(DownloadItem*);
    void resumeDownload(DownloadTask*);
    void onTaskPaused(DownloadTask*);
    void chackWhatStatus(DownloadTask::Status);
signals:
    void taskStarted(DownloadTask*);
    void taskFinished(DownloadTask*);
    void taskPaused(DownloadTask*);
private:
    int m_maxThread;
    QVector<QThread*> m_idleThreads;
    QHash<QThread*, DownloadTask*> m_busyThreads;

    QQueue<DownloadTask*> m_pendingQueue;

    void resumeTask(DownloadItem*);
    void removeFromPendingQueue(const QString&);
    void startNewTask(DownloadTask*);
    void returnThreadToPool(QThread*);
    void startNextTask();

    void calculateMaxThreads();

};

#endif // THREADPOOL_H
