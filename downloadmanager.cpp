#include "downloadmanager.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent){
    m_threadPool = new ThreadPool(this);
    m_db = new DownloadDatabase(this);
}

void DownloadManager::processDownloadRequest(const QString &url, const QString &saveDir, const DownloadTypes::UserChoice &userChoice){
    QString filePath = createDownloadPath(url, saveDir, userChoice.newFileName);
    QString fileName = !userChoice.newFileName.isEmpty() ? userChoice.newFileName : createDownloadFileName(url);

    DownloadTypes::ConflictResult result = checkForConflicts(url, saveDir, fileName);

    if(result.type == DownloadTypes::NoConflict || userChoice.action == DownloadTypes::Download ||
        (userChoice.action == DownloadTypes::DownloadWithNewName && result.type == DownloadTypes::UrlDownloading)){
        createAndStartDownload(url, filePath, fileName);
    }else if(userChoice.action == DownloadTypes::Cancel){

    }else{
        emit conflictsDetected(url, result);
    }
}

DownloadTypes::ConflictResult DownloadManager::checkForConflicts(const QString &url, const QString &saveDir, const QString &suggestedName)
{
    DownloadTypes::ConflictResult result;

    result.filePath = createDownloadPath(url, saveDir, suggestedName);

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

QString DownloadManager::createDownloadFileName(const QString &url)
{
    QString fileName;

    QUrl qurl(url);
    QString path = qurl.path();
    if (!path.isEmpty()) {
        QFileInfo info(path);
        fileName = info.fileName();
    }

    if (fileName.isEmpty()) {
        fileName = QString("download_%1")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    }
    return fileName;
}


QString DownloadManager::createDownloadPath(const QString &url,
                                            const QString &saveDir,
                                            const QString &suggestedName) {
    QDir dir(saveDir);

    QString fileName;

    if (!suggestedName.isEmpty()) {
        fileName = suggestedName;
        QUrl qurl(url);
        QString path = qurl.path();
        QFileInfo info(path);
        QFileInfo currentIngo(fileName);
        if(currentIngo.suffix().isEmpty()){
            fileName += ("." + info.suffix());
        }
    } else {
        fileName = createDownloadFileName(url);
    }

    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().isEmpty()) {
        fileName += ".bin";
    }

    return dir.absoluteFilePath(fileName);
}

void DownloadManager::createAndStartDownload(const QString &url, const QString &filePath, const QString& nameOfFile) {
    DownloadItem *item = new DownloadItem(url, filePath, nameOfFile);
    m_items.push_back(item);

    startDownload(item);

    emit downloadReadyToAdd(item);
}

void DownloadManager::startDownload(DownloadItem *item){
    DownloadTask *task = new DownloadTask(item->getUrl(), item->getFilePath());

    connect(item, &DownloadItem::statusChanged, task, &DownloadTask::setStatus, Qt::QueuedConnection);
    connect(task, &DownloadTask::progressChanged, item, &DownloadItem::onProgressChanged, Qt::QueuedConnection);
    connect(task, &DownloadTask::statusChanged, m_threadPool, &ThreadPool::chackWhatStatus, Qt::QueuedConnection);
    connect(task, &DownloadTask::statusChanged, item, &DownloadItem::chackWhatStatus, Qt::QueuedConnection);
    connect(item, &DownloadItem::deleteDownload, m_db, &DownloadDatabase::deleteDownload);
    connect(item, &DownloadItem::finishedDownload, this, &DownloadManager::finished);

    connect(item, &DownloadItem::ChangedBt, this, &DownloadManager::changeBt);

    m_threadPool->addTask(task);

    m_itemTask[item] = task;
    m_urlsDownloading.append(item->getUrl());
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
        DownloadTask* task = new DownloadTask(record.m_url, record.m_filePath);

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

void DownloadManager::saveAll(){
    for(auto item : m_items){
        QPair<DownloadItem*, DownloadTask*> pair(item, m_itemTask[item]);
        m_db->saveDownload(DownloadAdapter::toRecord(pair));
    }
}

DownloadManager::~DownloadManager(){}
