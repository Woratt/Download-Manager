#include "../headers/downloadmanager.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent){
    m_threadPool = new ThreadPool(this);
    m_db = new DownloadDatabase(this);
    m_networkManager = new NetworkManager(this);

    connect(m_threadPool, &ThreadPool::allDownloadsStoped, this, [this](){
        QVector<QPair<DownloadItem*, DownloadTask*>> pairs;
        for(auto it = m_itemTask.begin(); it != m_itemTask.end(); ++it){
            pairs.append(QPair<DownloadItem*, DownloadTask*>(it.key(), it.value()));
        }
        m_db->saveDownloads(DownloadAdapter::toRecord(pairs));
    }, Qt::QueuedConnection);

    connect(m_db, &DownloadDatabase::saveSuccesed, this, [&](){
        emit readyToQuit();
    });
}

void DownloadManager::processDownloadRequest(const QString &url, const QString &saveDir, const DownloadTypes::UserChoice &userChoice){

    connect(m_networkManager, &NetworkManager::fileInfoReady, this, [=](const RemoteFileInfo &info) {
        if (!info.isValid) {
            return;
        }

        QString finalFileName = !userChoice.newFileName.isEmpty() ? userChoice.newFileName : info.fileName;
        QString filePath = QDir(saveDir).absoluteFilePath(finalFileName);

        DownloadTypes::ConflictResult result = checkForConflicts(url, filePath);

        if(result.type == DownloadTypes::NoConflict || userChoice.action == DownloadTypes::Download ||
            (userChoice.action == DownloadTypes::DownloadWithNewName && result.type == DownloadTypes::UrlDownloading)){
            createAndStartDownload(url, filePath, finalFileName, info.fileSize);
        } else if(userChoice.action == DownloadTypes::Cancel) {

        }else{
            emit conflictsDetected(url, result);
        }

    }, Qt::SingleShotConnection);

    m_networkManager->getRemoteFileInfo(QUrl(url));
}

DownloadTypes::ConflictResult DownloadManager::checkForConflicts(const QString &url, const QString &filePuth)
{
    DownloadTypes::ConflictResult result;

    result.filePath = filePuth;

    qDebug() << result.filePath;

    bool fileExists = QFile::exists(result.filePath);

    result.existingDownloads = m_urlsDownloading.contains(url);

    if (fileExists && result.existingDownloads) {
        result.type = DownloadTypes::BothConflicts;
    } else if (fileExists) {
        result.type = DownloadTypes::FileExists;
    } else if (result.existingDownloads) {
        result.type = DownloadTypes::UrlDownloading;
    } else {
        result.type = DownloadTypes::NoConflict;
    }
    return result;
}

void DownloadManager::createAndStartDownload(const QString &url, const QString &filePath, const QString& nameOfFile, qint64 fileSize) {
    DownloadItem *item = new DownloadItem(url, filePath, nameOfFile);
    DownloadTask *task = new DownloadTask(url, filePath, fileSize);
    m_items.push_back(item);

    connect(item, &DownloadItem::statusChanged, task, &DownloadTask::setStatus, Qt::QueuedConnection);
    connect(task, &DownloadTask::progressChanged, item, &DownloadItem::onProgressChanged, Qt::QueuedConnection);
    connect(task, &DownloadTask::statusChanged, m_threadPool, &ThreadPool::chackWhatStatus, Qt::QueuedConnection);
    connect(task, &DownloadTask::statusChanged, item, &DownloadItem::chackWhatStatus, Qt::QueuedConnection);
    connect(item, &DownloadItem::deleteDownload, m_db, &DownloadDatabase::deleteDownload);
    connect(item, &DownloadItem::finishedDownload, this, &DownloadManager::finished);

    connect(item, &DownloadItem::ChangedBt, this, &DownloadManager::changeBt);

    m_threadPool->addTask(task);

    m_itemTask[item] = task;
    m_urlsDownloading.append(url);

    emit downloadReadyToAdd(item);
}

void DownloadManager::finished(){
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    m_urlsDownloading.removeOne(item->getUrl());
}

void DownloadManager::changeBt(DownloadItem* item, bool checked){
    if(checked && !m_selectedItems.contains(item)){
        m_selectedItems.push_back(item);
    }else if(!checked && m_selectedItems.contains(item)){
        m_selectedItems.removeOne(item);
    }

    if(m_selectedItems.size() > 0){
        emit showButtons();
    }else{
        emit hideButtons();
    }
}

void DownloadManager::downloadAll(){
    for(auto* item : m_selectedItems){
        item->pauseDownloadAll(true);
        item->setNotChecked();
    }
    m_selectedItems.clear();
    emit hideButtons();
}

void DownloadManager::pauseAll(){
    for(auto* item : m_selectedItems){
        item->pauseDownloadAll(false);
        item->setNotChecked();
    }
    m_selectedItems.clear();
    emit hideButtons();
}

void DownloadManager::deleteAll(){
    for(auto* item : m_selectedItems){
        item->deleteItem();
    }
    m_selectedItems.clear();
    emit hideButtons();
}

void DownloadManager::setItemsFromDB(){
    QVector<DownloadRecord> records = m_db->getDownloads();

    qDebug() << "downloads size: " << records.size();
    for(auto& record : records){
        DownloadItem* item = new DownloadItem(record.m_url, record.m_filePath, record.m_name);
        DownloadTask* task = new DownloadTask(record.m_url, record.m_filePath, 0);

        m_items.push_back(item);

        item->updateFromDb(record);
        task->updateFromDb(record);

        connect(item, &DownloadItem::statusChanged, task, &DownloadTask::setStatus, Qt::QueuedConnection);
        connect(task, &DownloadTask::progressChanged, item, &DownloadItem::onProgressChanged, Qt::QueuedConnection);
        connect(task, &DownloadTask::statusChanged, m_threadPool, &ThreadPool::chackWhatStatus, Qt::QueuedConnection);
        connect(task, &DownloadTask::statusChanged, item, &DownloadItem::chackWhatStatus, Qt::QueuedConnection);
        connect(item, &DownloadItem::deleteDownload, m_db, &DownloadDatabase::deleteDownload);
        connect(item, &DownloadItem::finishedDownload, this, &DownloadManager::finished);

        connect(item, &DownloadItem::ChangedBt, this, &DownloadManager::changeBt);

        m_threadPool->addTaskFromDB(task);

        m_itemTask[item] = task;
        m_urlsDownloading.append(item->getUrl());

        emit downloadReadyToAdd(item);
    }
}

void DownloadManager::prepareToExit(){
    QVector<DownloadTask*> tasks;

    for(auto task : m_itemTask){
        if(task->getStatus() == DownloadTask::Status::Downloading || task->getStatus() == DownloadTask::Status::ResumedInDownloading
            || task->getStatus() == DownloadTask::Status::Resumed){
            tasks.append(task);
        }
    }
    m_threadPool->stopAllDownloads(tasks);
}

DownloadManager::~DownloadManager(){}
