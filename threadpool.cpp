#include "threadpool.h"

ThreadPool::ThreadPool(int maxThread, QObject *parent) : QObject(parent), m_maxThread(maxThread) {
    for(int i = 0; i < m_maxThread; ++i){
        QThread *thread = new QThread(this);
        thread->start();
        m_idleThreads.append(thread);
    }
}

void ThreadPool::addTask(DownloadTask *task){
    /*qDebug() << "m_activeTasks size: " << m_activeTasks.size();
    if(m_activeTasks.contains(item->getUrl())){
        qDebug() << "Download already in progress:" << item->getUrl();
        return;
    }

    if(m_activeTasks.size() >= m_maxThread){
        m_pendingQueue.enqueue(item);
        item->setStatus("pending");
        return;
    }

    startNewTask(item);*/

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

    for (auto t : m_pendingQueue) {
        if (t == task) {
            qDebug() << "Task already in pending queue";
            return;
        }
    }

    if(m_busyThreads.size() == m_maxThread)
    {
        m_pendingQueue.enqueue(task);
    }else
    {
        startNextTask();
    }

}

void ThreadPool::startNextTask(){
    if(m_idleThreads.isEmpty() || m_pendingQueue.isEmpty())
    {
        return;
    }

    QThread *thread = m_idleThreads.takeFirst();
    DownloadTask *task = m_pendingQueue.dequeue();

    task->moveToThread(thread);
    m_busyThreads[thread] = task;

    connect(task, &DownloadTask::finished, this, &ThreadPool::onTaskFinished);
    connect(task, &DownloadTask::paused, this, &ThreadPool::onTaskPaused);

    QMetaObject::invokeMethod(task, "startDownload", Qt::QueuedConnection);

    emit taskStarted(task);
}

void ThreadPool::onTaskPaused(){

}


void ThreadPool::startNewTask(DownloadItem* item){
    /*DownloadTask* task = new DownloadTask(item->getUrl(), item->getFilePath());
    QThread* thread = new QThread(this);

    setUpConnections(thread, task, item);

    qDebug() << "Status: " << item->getStatus();

    if(item->isFromDB()){
        if(item->getStatus() == "pending"){
            //m_pendingQueue.enqueue(item);
            return;
        }else if(item->getStatus() == "paused"){
            item->pauseDownloadAll(false);
        }else if(item->getStatus() == "completed"){
            item->pauseDownloadAll(false);
            task->deleteLater();
            thread->deleteLater();
            return;
        }else{
            task->resumeFromDB(thread, item->getResumePos());
        }
    }else{
        task->startNewTask(thread);
    }

    m_activeTasks[item->getUrl()] = QPair<QThread*, DownloadTask*>(thread, task);
    thread->start();*/
}

void ThreadPool::setUpConnections(QThread* thread, DownloadTask* task, DownloadItem* item){
    /*connect(task, &DownloadTask::finished, this, &ThreadPool::onTaskFinished, Qt::QueuedConnection);

    connect(task, &DownloadTask::progressChanged, item, &DownloadItem::onProgressChanged, Qt::QueuedConnection);
    connect(task, &DownloadTask::finished, item, &DownloadItem::onFinished, Qt::QueuedConnection);
    connect(item, &DownloadItem::pauseDownload, task, &DownloadTask::pauseDownload, Qt::QueuedConnection);
    connect(item, &DownloadItem::resumeDownload, task, &DownloadTask::resumeDownload, Qt::QueuedConnection);
    connect(item, &DownloadItem::deleteDownload, this, &ThreadPool::onDeleteRequested);

    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(task, &DownloadTask::finished, task, &QObject::deleteLater);*/

}

void ThreadPool::onTaskFinished(const QString& url){
    DownloadTask *task = qobject_cast<DownloadTask*>(sender());
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

        disconnect(task, &DownloadTask::finished, this, &ThreadPool::onTaskFinished);
        disconnect(task, &DownloadTask::paused, this, &ThreadPool::onTaskPaused);
    }

    emit taskFinished(task);

    startNextTask();
    /*auto it = m_activeTasks.find(url);

    if(it != m_activeTasks.end()){
        QThread* thread = it.value().first;

        if(thread && thread->isRunning()){
            thread->quit();
        }
    }
    m_activeTasks.erase(it);

    startNewPendingTask();*/
}

void ThreadPool::returnThreadToPool(QThread* thread)
{
    if(thread)
    {
        return;
    }

    if (thread->isRunning()) {
        thread->quit();
        thread->wait(500);
    }

    if(!m_idleThreads.contains(thread))
    {
        m_idleThreads.append(thread);
    }

}

void ThreadPool::onDeleteRequested(DownloadItem* item){
    /*QString url = item->getUrl();
    auto it = m_activeTasks.find(url);

    if(it != m_activeTasks.end()){
        QThread *thread = it.value().first;
        DownloadTask *task = it.value().second;

        if(task){
            task->stopDownload();
            task->deleteLater();
        }
        qDebug() << "m_activeTasks: " << m_activeTasks;
        if(thread && thread->isRunning()){
            thread->quit();
            thread->wait(1000);
            if(thread->isRunning()) {
                thread->terminate();
                thread->wait();
            }
        }
    }

    removeFromPendingQueue(url);*/
}

void ThreadPool::startNewPendingTask(){
    qDebug() << "m_pendingQueue size: " << m_pendingQueue.size();
    qDebug() << "m_activeTasks size: " << m_activeTasks.size();
    if(!m_pendingQueue.isEmpty()){
        //DownloadItem *item = m_pendingQueue.dequeue();
        //item->setStatus("downloading");
        //startNewTask(item);
    }
}

void ThreadPool::removeFromPendingQueue(const QString& url){
    QQueue<DownloadItem*> newQueue;
    while(!m_pendingQueue.isEmpty()){
        //DownloadItem *item = m_pendingQueue.dequeue();
        //if(item->getUrl() == url){
            //item->deleteLater();
        //}else{
            //newQueue.enqueue(item);
        //}
    }
    //m_pendingQueue = newQueue;
    //startNewPendingTask();
}

ThreadPool::~ThreadPool(){
    /*for(auto& activeTask : m_activeTasks){
        activeTask.first->quit();
        activeTask.first->deleteLater();
    }
    for(auto& pendingQueue : m_pendingQueue){
        pendingQueue->deleteLater();
    }*/
}


