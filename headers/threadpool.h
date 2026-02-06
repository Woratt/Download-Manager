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
    void addTask(std::shared_ptr<DownloadTask> task);
    void addTaskFromDB(std::shared_ptr<DownloadTask> task);
    void stopAllDownloads(QVector<std::shared_ptr<DownloadTask>>& tasks);
    void removeTask(std::shared_ptr<DownloadTask>);
    ~ThreadPool();
signals:
    void allDownloadsStoped();
public slots:
    void onTaskFinished(std::shared_ptr<DownloadTask> task);
    void resumeDownload(std::shared_ptr<DownloadTask> task);
    void onTaskPaused(std::shared_ptr<DownloadTask> task);
    void chackWhatStatus(DownloadTask::Status status);
private:
    mutable QRecursiveMutex m_mutex;
    int m_maxThread;
    QVector<QThread*> m_idleThreads;
    QHash<QThread*, std::shared_ptr<DownloadTask>> m_busyThreads;

    QQueue<std::shared_ptr<DownloadTask>> m_pendingQueue;

    void startNewTask(std::shared_ptr<DownloadTask> task);
    void returnThreadToPool(QThread*);
    void startNextTask();

    void calculateMaxThreads();
};

#endif // THREADPOOL_H
