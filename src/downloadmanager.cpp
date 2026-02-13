#include "../headers/downloadmanager.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent){
    m_threadPool = new ThreadPool(this);
    m_db = new DownloadDatabase("",this);
    m_networkManager = new NetworkManager(this);
    m_storageManager = new StorageManager();

    m_storageThread = new QThread(this);

    m_storageManager->moveToThread(m_storageThread);

    connect(m_storageThread, &QThread::finished, m_storageManager, &QObject::deleteLater);

    m_storageThread->start();

    connect(m_threadPool, &ThreadPool::allDownloadsStoped, this, [this](){
        QVector<QPair<DownloadItem*, std::shared_ptr<DownloadTask>>> pairs;
        for(auto it = m_itemTask.begin(); it != m_itemTask.end(); ++it){
            pairs.append(QPair<DownloadItem*, std::shared_ptr<DownloadTask>>(it.key(), it.value()));
        }
        m_db->saveDownloads(DownloadAdapter().toRecords(pairs));
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

        QString finalFileName = !userChoice.newFileName.isEmpty() ? userChoice.newFileName + info.suffix : info.fileName;
        QString filePath = QDir(saveDir).absoluteFilePath(finalFileName);

        DownloadTypes::ConflictResult result = checkForConflicts(info.url.toString(), filePath);

        if(result.type == DownloadTypes::NoConflict || userChoice.action == DownloadTypes::Download ||
            (userChoice.action == DownloadTypes::DownloadWithNewName && result.type == DownloadTypes::UrlDownloading)){
            createAndStartDownload(info.url.toString(), filePath, finalFileName, info.fileSize);
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
    DownloadTypes::FileInfo fileInfo;
    fileInfo.fileName = nameOfFile;
    fileInfo.filePath = filePath;
    fileInfo.fileSize = fileSize;

    DownloadItem *item = new DownloadItem(url, filePath, nameOfFile);
    std::shared_ptr<DownloadTask> task = std::make_shared<DownloadTask>(url, fileInfo);

    QMetaObject::invokeMethod(m_storageManager, "openFile",
                              Qt::QueuedConnection,
                              Q_ARG(DownloadTypes::FileInfo, fileInfo));

    m_items.push_back(item);

    connect(m_storageManager, &StorageManager::savedLastChunk, task.get(), &DownloadTask::onFinished, Qt::QueuedConnection);
    connect(m_storageManager, &StorageManager::changeQuantityOfChunks, task.get(), &DownloadTask::changeQuantityOfChunks, Qt::QueuedConnection);
    connect(task.get(), &DownloadTask::openFile, m_storageManager, &StorageManager::openFile, Qt::QueuedConnection);
    connect(task.get(), &DownloadTask::clearFile, m_storageManager, &StorageManager::clearFile, Qt::QueuedConnection);
    connect(task.get(), &DownloadTask::stopWrite, m_storageManager, &StorageManager::closeFile, Qt::QueuedConnection);
    connect(task.get(), &DownloadTask::writeChunk, m_storageManager, &StorageManager::writeChunk, Qt::QueuedConnection);
    connect(item, &DownloadItem::statusChanged, task.get(), &DownloadTask::setStatus, Qt::QueuedConnection);
    connect(task.get(), &DownloadTask::progressChanged, item, &DownloadItem::onProgressChanged, Qt::QueuedConnection);
    connect(task.get(), &DownloadTask::statusChanged, m_threadPool, &ThreadPool::chackWhatStatus, Qt::QueuedConnection);
    connect(task.get(), &DownloadTask::statusChanged, item, &DownloadItem::chackWhatStatus, Qt::QueuedConnection);
    connect(item, &DownloadItem::deleteDownload, this, &DownloadManager::deleteDownload);
    connect(item, &DownloadItem::finishedDownload, this, &DownloadManager::finished);
    connect(item, &DownloadItem::ChangedBt, this, &DownloadManager::changeBt);

    connect(m_storageManager, &StorageManager::fileOpen, this, [=](const DownloadTypes::FileInfo &fileInfo){
        if(fileInfo == task->getFileInfo()){

            m_threadPool->addTask(task);

            m_itemTask[item] = task;
            m_urlsDownloading.append(url);

            emit downloadReadyToAdd(item);
        }
    }, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection | Qt::QueuedConnection));

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

    for(auto& record : records){
        DownloadTypes::FileInfo fileInfo;
        fileInfo.fileName = record.m_name;
        fileInfo.filePath = record.m_filePath;
        fileInfo.fileSize = record.m_totalBytes;
        DownloadItem* item = new DownloadItem(record.m_url, record.m_filePath, record.m_name);
        std::shared_ptr<DownloadTask> task = std::make_shared<DownloadTask>(record.m_url, fileInfo);

        m_items.push_back(item);

        item->updateFromDb(record);
        task->updateFromDb(record);

        connect(m_storageManager, &StorageManager::fileOpen, this, [=](const DownloadTypes::FileInfo &fileInfo){
            if(fileInfo == task->getFileInfo() && !m_itemTask[item]){

                m_threadPool->addTaskFromDB(task);

                m_itemTask[item] = task;
                m_urlsDownloading.append(record.m_url);

                emit downloadReadyToAdd(item);
            }
        }, Qt::QueuedConnection);

        QMetaObject::invokeMethod(m_storageManager, "openFile",
                                  Qt::QueuedConnection,
                                  Q_ARG(DownloadTypes::FileInfo, fileInfo));

        connect(m_storageManager, &StorageManager::savedLastChunk, task.get(), &DownloadTask::onFinished, Qt::QueuedConnection);
        connect(m_storageManager, &StorageManager::changeQuantityOfChunks, task.get(), &DownloadTask::changeQuantityOfChunks, Qt::QueuedConnection);
        connect(task.get(), &DownloadTask::openFile, m_storageManager, &StorageManager::openFile, Qt::QueuedConnection);
        connect(task.get(), &DownloadTask::clearFile, m_storageManager, &StorageManager::clearFile, Qt::QueuedConnection);
        connect(task.get(), &DownloadTask::stopWrite, m_storageManager, &StorageManager::closeFile, Qt::QueuedConnection);
        connect(task.get(), &DownloadTask::writeChunk, m_storageManager, &StorageManager::writeChunk, Qt::QueuedConnection);
        connect(item, &DownloadItem::statusChanged, task.get(), &DownloadTask::setStatus, Qt::QueuedConnection);
        connect(task.get(), &DownloadTask::progressChanged, item, &DownloadItem::onProgressChanged, Qt::QueuedConnection);
        connect(task.get(), &DownloadTask::statusChanged, m_threadPool, &ThreadPool::chackWhatStatus, Qt::QueuedConnection);
        connect(task.get(), &DownloadTask::statusChanged, item, &DownloadItem::chackWhatStatus, Qt::QueuedConnection);
        connect(item, &DownloadItem::deleteDownload, this, &DownloadManager::deleteDownload);
        connect(item, &DownloadItem::finishedDownload, this, &DownloadManager::finished);

        connect(item, &DownloadItem::ChangedBt, this, &DownloadManager::changeBt);
    }
}

void DownloadManager::prepareToExit(){
    QVector<std::shared_ptr<DownloadTask>> tasks;

    for(auto task : m_itemTask){
        if(task->getStatus() == DownloadTask::Status::Downloading || task->getStatus() == DownloadTask::Status::ResumedInDownloading
            || task->getStatus() == DownloadTask::Status::Resumed){
            tasks.append(task);
        }
    }
    m_threadPool->stopAllDownloads(tasks);
}

void DownloadManager::deleteDownload(DownloadItem *item){
    if (!item) return;

    std::shared_ptr<DownloadTask> task = m_itemTask.value(item);

    if (task) {
        m_threadPool->removeTask(task);

        QMetaObject::invokeMethod(m_storageManager, "deleteAllInfo",
                                  Qt::QueuedConnection,
                                  Q_ARG(DownloadTypes::FileInfo, task->getFileInfo()));

        task->disconnect();

        //task->deleteLater();
    }

    m_itemTask.remove(item);
    m_items.removeOne(item);
    m_db->deleteDownload(DownloadAdapter().toRecord(item, task));
    m_urlsDownloading.removeOne(item->getUrl());
    emit deleteDownloadItem(item);
}

DownloadManager::~DownloadManager(){
    m_storageThread->quit();
    m_storageThread->wait();
    m_storageThread->deleteLater();
}
