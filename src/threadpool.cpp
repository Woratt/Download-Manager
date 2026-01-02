#include "../headers/threadpool.h"

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
    if (!task) {
        qWarning() << "Cannot add null task";
        return;
    }

    for (auto t : m_busyThreads.values()) {
        if (t == task) {
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

void ThreadPool::addTaskFromDB(DownloadTask* task){
    DownloadTask::Status status = task->getStatus();
    switch (status) {
    case DownloadTask::Status::Resumed:
        resumeDownload(task);
        break;
    case DownloadTask::Status::Completed:
        onTaskFinished(task);
        break;
    case DownloadTask::Status::Error:
        onTaskFinished(task);
        break;
    case DownloadTask::Status::Cancelled:
        onTaskFinished(task);
        break;
    case DownloadTask::Status::Paused:
        onTaskPaused(task);
        break;
    case DownloadTask::Status::Pending:
        m_pendingQueue.enqueue(task);
        break;
    case DownloadTask::Status::StartNewTask:
        startNewTask(task);
        break;
    case DownloadTask::Status::Deleted:
        onTaskFinished(task);
        break;
    case DownloadTask::Status::Downloading:
        resumeDownload(task);
        break;
    case DownloadTask::Status::ResumedInDownloading:
        resumeDownload(task);
        break;
    case DownloadTask::Status::PausedResume:
        resumeDownload(task);
        break;
    case DownloadTask::Status::ResumedInPending:
        resumeDownload(task);
        break;
    case DownloadTask::Status::PausedNew:
        onTaskPaused(task);
        break;
    }

}

void ThreadPool::startNewTask(DownloadTask* task){
    if(m_idleThreads.isEmpty())
    {
        return;
    }

    if (task->thread() != this->thread()) {
        QMetaObject::invokeMethod(task, [task, this]() {
            task->moveToThread(this->thread());
        }, Qt::BlockingQueuedConnection);
    }

    QThread *thread = m_idleThreads.takeFirst();

    task->moveToThread(thread);
    m_busyThreads[thread] = task;

    QMetaObject::invokeMethod(task, "startDownload", Qt::QueuedConnection);
}

void ThreadPool::resumeDownload(DownloadTask* task){

    if (task->thread() != this->thread()) {
        QMetaObject::invokeMethod(task, [task, this]() {
            task->moveToThread(this->thread());
        }, Qt::BlockingQueuedConnection);
    }

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
}

/*bool ThreadPool::isAllDownloadsFree(){
    if(m_maxThread == m_idleThreads.size()) return true;
    else return false;
}*/

void ThreadPool::stopAllDownloads(QVector<DownloadTask*>& tasks){
    if (tasks.isEmpty()) {
        emit allDownloadsStoped();
        return;
    }

    auto remaining = std::make_shared<int>(tasks.size());
    for (DownloadTask* taskPtr : tasks) { // Використовуємо range-based for
        if (!taskPtr) {
            (*remaining)--;
            continue;
        }

        QThread *thread = nullptr;
        // Шукаємо потік для цього таска
        for (auto it = m_busyThreads.begin(); it != m_busyThreads.end(); ++it) {
            if (it.value() == taskPtr) {
                thread = it.key();
                break;
            }
        }

        if (thread) {
            // Захоплюємо taskPtr (вказівник), а не ітератор!
            connect(taskPtr, &DownloadTask::stoped, this, [this, taskPtr, thread, remaining]() {

                // Перевірка, чи об'єкт ще живий (опціонально, але бажано)
                if (taskPtr) {
                    taskPtr->moveToThread(this->thread());
                }

                this->returnThreadToPool(thread);
                this->m_busyThreads.remove(thread);

                // Якщо ми закриваємо програму, startNextTask() зазвичай не потрібен,
                // але залишимо для логіки "паузи всього"
                // this->startNextTask();

                (*remaining)--;
                if (*remaining <= 0) {
                    emit allDownloadsStoped();
                }
            }, Qt::QueuedConnection);

            QMetaObject::invokeMethod(taskPtr, "stopDownload", Qt::QueuedConnection);
        } else {
            // Якщо таск не був у m_busyThreads, він уже стоїть
            (*remaining)--;
        }
    }

    // Якщо раптом усі таски були нульові або не в потоках
    if (*remaining <= 0) {
        emit allDownloadsStoped();
    }

    /*for(auto task = tasks.begin(); task != tasks.end(); ++task){
        if (!*task) continue;

        QThread *thread = nullptr;
        for(auto it = m_busyThreads.begin(); it != m_busyThreads.end(); ++it)
        {
            if(it.value() == *task)
            {
                thread = it.key();
                break;
            }
        }

        if(thread)
        {

            connect(*task, &DownloadTask::stoped, this, [=]() {
                    (*task)->moveToThread(this->thread());
                    this->returnThreadToPool(thread);
                    this->m_busyThreads.remove(thread);
                    this->startNextTask();
                    if(task == (tasks.end() - 1)){
                        emit allDownloadsStoped();
                    }
            }, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::SingleShotConnection));

            QMetaObject::invokeMethod(*task, "stopDownload", Qt::QueuedConnection);

        }
    }*/
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

        connect(task, &DownloadTask::paused, this, [this, task, thread]() {
            if (task->getStatus() == DownloadTask::Status::Paused) {
                task->moveToThread(this->thread());
                this->returnThreadToPool(thread);
                this->m_busyThreads.remove(thread);
                this->startNextTask();
            }
        }, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::SingleShotConnection));

        QMetaObject::invokeMethod(task, "pauseDownload", Qt::QueuedConnection);

    }
}

void ThreadPool::chackWhatStatus(DownloadTask::Status status){
    DownloadTask* task = qobject_cast<DownloadTask*>(sender());

    if (task->thread() != this->thread()) {
        QMetaObject::invokeMethod(task, [task, this]() {
            task->moveToThread(this->thread());
        }, Qt::BlockingQueuedConnection);
    }

    switch (status) {
    case DownloadTask::Status::Resumed:
        resumeDownload(task);
        qDebug() << "Resumed";
        break;
    case DownloadTask::Status::Completed:
        onTaskFinished(task);
        qDebug() << "Completed";
        break;
    case DownloadTask::Status::Error:
        onTaskFinished(task);;
        qDebug() << "Error";
        break;
    case DownloadTask::Status::Cancelled:
        onTaskFinished(task);
        qDebug() << "Cancelled";
        break;
    case DownloadTask::Status::Paused:
        onTaskPaused(task);
        qDebug() << "Paused";
        break;
    case DownloadTask::Status::Pending:
        m_pendingQueue.enqueue(task);
        qDebug() << "Pending";
        break;
    case DownloadTask::Status::StartNewTask:
        startNewTask(task);
        break;
    case DownloadTask::Status::Deleted:
        onTaskFinished(task);
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

        if (task->thread() != this->thread()) {
            QMetaObject::invokeMethod(task, [task, this]() {
                task->moveToThread(this->thread());
            }, Qt::BlockingQueuedConnection);
            QCoreApplication::processEvents();
        }

        returnThreadToPool(thread);
        m_busyThreads.remove(thread);

    }

    startNextTask();
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

    m_maxThread = qBound(2, static_cast<int>(cores), 8);
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


