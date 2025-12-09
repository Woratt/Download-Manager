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
    explicit ThreadPool(int maxThread = 2, QObject *parent = nullptr);
    void addTask(DownloadTask*);
    ~ThreadPool();
public slots:
    void onTaskFinished(const QString&);
    void onDeleteRequested(DownloadItem*);
    void onTaskPaused();
signals:
    void taskStarted(DownloadTask*);
    void taskFinished(DownloadTask*);
    void taskPaused(DownloadTask*);
private:
    int m_maxThread;
    QHash<QString, QPair<QThread*, DownloadTask*>> m_activeTasks;
    //QQueue<DownloadItem*> m_pendingQueue;

    QVector<QThread*> m_idleThreads;
    QHash<QThread*, DownloadTask*> m_busyThreads;

    QQueue<DownloadTask*> m_pendingQueue;

    void startNewTask(DownloadItem*);
    void resumeTask(DownloadItem*);
    void setUpConnections(QThread*, DownloadTask*, DownloadItem*);
    void removeFromPendingQueue(const QString&);
    void startNewPendingTask();
    void startNextTask();
    void returnThreadToPool(QThread*);
};

#endif // THREADPOOL_H
