#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QObject>
#include <QHash>
#include <QPair>
#include <QThread>
#include <QQueue>
#include <QMutex>

#include "downloaditem.h"
#include "downloadtask.h"

class ThreadPool : public QObject
{
    Q_OBJECT
public:
    explicit ThreadPool(QObject *parent = nullptr);
    void addTask(DownloadTask*);
    void addTaskFromDB(DownloadTask*);
    void stopAllDownloads(QVector<DownloadTask*>&);
    ~ThreadPool();
signals:
    void allDownloadsStoped();
public slots:
    void onTaskFinished(DownloadTask*);
    void resumeDownload(DownloadTask*);
    void onTaskPaused(DownloadTask*);
    void chackWhatStatus(DownloadTask::Status);
private:
    mutable QRecursiveMutex m_mutex;
    int m_maxThread;
    QVector<QThread*> m_idleThreads;
    QHash<QThread*, DownloadTask*> m_busyThreads;

    QQueue<DownloadTask*> m_pendingQueue;

    void startNewTask(DownloadTask*);
    void returnThreadToPool(QThread*);
    void startNextTask();

    void calculateMaxThreads();
};

#endif // THREADPOOL_H
