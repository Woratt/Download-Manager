#include "threadpool.h"

ThreadPool::ThreadPool(QObject *parent) : QObject(parent)
{
    calculateMaxThreads();
    for(int i = 0; i < m_maxThread; ++i){
        QThread *thread = new QThread(this);
        thread->start();
        m_idleThreads.append(thread);
    }
}

void ThreadPool::addTask(DownloadTask *task){
    qDebug() << "m_idleThreads size: "  << m_idleThreads.size();

    if (!task) {
        qWarning() << "Cannot add null task";
        return;
    }

    for (auto t : m_busyThreads.values()) {
        if (t == task) {
            qDebug() << "Task already running";
            return;
        }
    }

    if(!m_idleThreads.isEmpty())
    {
        startNewTask(task);
    }else
    {
        m_pendingQueue.enqueue(task);
    }
}

void ThreadPool::isStartOrResume()
{
    /*qDebug() << "m_pendingQueue size: " << m_pendingQueue.size() << "m_idleThreads size: " << m_idleThreads.size();
    if(m_pendingQueue.isEmpty() || m_idleThreads.isEmpty())
    {
        return;
    }
    DownloadTask* task = m_pendingQueue.dequeue();
    if(task->isStartedDownload())
    {
        resumeDownload(task);
    }else
    {
        startNextTask(task);
    }*/
}

void ThreadPool::startNewTask(DownloadTask* task){
    if(m_idleThreads.isEmpty())
    {
        return;
    }

    QThread *thread = m_idleThreads.takeFirst();

    task->moveToThread(thread);
    m_busyThreads[thread] = task;

    QMetaObject::invokeMethod(task, "startDownload", Qt::QueuedConnection);

    emit taskStarted(task);
}

void ThreadPool::resumeDownload(DownloadTask* task){
    if(m_idleThreads.isEmpty())
    {
        task->setStatus(DownloadTask::Status::ResumedInPending);
        m_pendingQueue.enqueue(task);
        return;
    }else
    {
        task->setStatus(DownloadTask::Status::ResumedInDownloading);
    }

    QThread *thread = m_idleThreads.takeFirst();
    task->moveToThread(thread);
    m_busyThreads[thread] = task;

    QMetaObject::invokeMethod(task, "resumeDownload", Qt::QueuedConnection);

    emit taskStarted(task);
}

void ThreadPool::onTaskPaused(DownloadTask *task){
    if (!task) return;


    QThread *thread = nullptr;
    for(auto it = m_busyThreads.begin(); it != m_busyThreads.end(); ++it)
    {
        if(it.value() == task)
        {
            thread = it.key();
            break;
        }
    }

    if(thread)
    {
        returnThreadToPool(thread);
        m_busyThreads.remove(thread);

        QMetaObject::invokeMethod(task, "pauseDownload", Qt::QueuedConnection);
    }

    emit taskPaused(task);

    //startNextTask();
}

void ThreadPool::chackWhatStatus(DownloadTask::Status status){
    DownloadTask* task = qobject_cast<DownloadTask*>(sender());
    switch (status) {
    case DownloadTask::Status::Resumed:
        resumeDownload(task);
        qDebug() << "Resumed";
        break;
    case DownloadTask::Status::Completed:
        onTaskFinished(task);
        startNextTask();
        qDebug() << "Completed";
        break;
    case DownloadTask::Status::Error:
        onTaskFinished(task);
        startNextTask();
        qDebug() << "Error";
        break;
    case DownloadTask::Status::Cancelled:
        onTaskFinished(task);
        startNextTask();
        qDebug() << "Cancelled";
        break;
    case DownloadTask::Status::Paused:
        onTaskPaused(task);
        startNextTask();
        qDebug() << "Paused";
        break;
    case DownloadTask::Status::Pending:
        m_pendingQueue.enqueue(task);
        qDebug() << "Pending";
        break;
    }
}

void ThreadPool::startNextTask()
{
    if(m_pendingQueue.isEmpty())
    {
        return;
    }
    DownloadTask *task = m_pendingQueue.dequeue();

    if(task->getStatus() == DownloadTask::Status::Pending)
    {
        startNewTask(task);
    }else if(task->getStatus() == DownloadTask::Status::ResumedInPending)
    {
        resumeDownload(task);
    }else
    {
        m_pendingQueue.enqueue(task);
    }
}

void ThreadPool::onTaskFinished(DownloadTask *task){
    if (!task) return;

    QThread *thread = nullptr;
    for(auto it = m_busyThreads.begin(); it != m_busyThreads.end(); ++it){
        if(it.value() == task){
            thread = it.key();
            break;
        }
    }

    if(thread){
        returnThreadToPool(thread);
        m_busyThreads.remove(thread);
    }

    emit taskFinished(task);
}

void ThreadPool::returnThreadToPool(QThread* thread)
{
    if(!thread)
    {
        return;
    }

    if(!m_idleThreads.contains(thread))
    {
        m_idleThreads.append(thread);
    }

}

void ThreadPool::calculateMaxThreads(){
    int cores = QThread::idealThreadCount();
    if (cores <= 0) cores = 2;

    m_maxThread = qBound(2, static_cast<int>(cores / 4), 8);
}

ThreadPool::~ThreadPool(){
    for(auto it = m_busyThreads.begin(); it != m_busyThreads.end(); ++it)
    {
        it.key()->quit();
        it.key()->wait();
        it.key()->deleteLater();
    }

    for(auto it : m_idleThreads)
    {
        it->quit();
        it->wait();
        it->deleteLater();
    }
}


