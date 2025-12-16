#include "downloadmanager.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent){
    m_threadPool = new ThreadPool(this);
    m_db = new DownloadDatabase(this);
}

void DownloadManager::processDownloadRequest(const QString &url, const QString &saveDir, const DownloadTypes::UserChoice &userChoice){
    QString filePath = createDownloadPath(url, saveDir, userChoice.newFileName);


    DownloadTypes::ConflictResult result = checkForConflicts(url, saveDir, userChoice.newFileName);

    if(result.type == DownloadTypes::NoConflict || userChoice.action == DownloadTypes::Download ||
        (userChoice.action == DownloadTypes::DownloadWithNewName && result.type == DownloadTypes::UrlDownloading)){
        createAndStartDownload(url, filePath, userChoice.newFileName);
    }else if(userChoice.action == DownloadTypes::Cancel){

    }else{
        emit conflictsDetected(url, result);
    }
}

DownloadTypes::ConflictResult DownloadManager::checkForConflicts(const QString &url, const QString &saveDir, const QString &suggestedName)
{
    DownloadTypes::ConflictResult result;

    result.filePath = createDownloadPath(url, saveDir, suggestedName);

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


QString DownloadManager::createDownloadPath(const QString &url,
                                            const QString &saveDir,
                                            const QString &suggestedName) {
    QDir dir(saveDir);

    // Визначити ім'я файлу
    QString fileName;

    if (!suggestedName.isEmpty()) {
        fileName = suggestedName;
        QUrl qurl(url);
        QString path = qurl.path();
        QFileInfo info(path);
        fileName += ("." + info.suffix());
    } else {
        // Спробувати витягти з URL
        QUrl qurl(url);
        QString path = qurl.path();
        if (!path.isEmpty()) {
            QFileInfo info(path);
            fileName = info.fileName();
        }

        // Якщо не вийшло - згенерувати
        if (fileName.isEmpty()) {
            fileName = QString("download_%1")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        }
    }

    // Перевірити розширення
    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().isEmpty()) {
        fileName += ".bin"; // або визначити по Content-Type
    }

    return dir.absoluteFilePath(fileName);
}



/*DownloadManager::ConflictInfo DownloadManager::checkConflicts(
    const QString &url, const QString &filePath) {

    ConflictInfo info;

    // 1. Перевірити файл
    info.fileExists = QFile::exists(filePath);

    // 2. Перевірити активні завантаження
    for (auto it = m_itemTask.begin(); it != m_itemTask.end(); ++it) {
        if (it.key()->getUrl() == url) {
            info.existingItems.append(it.key());
        }
    }

    return info;
}*/

void DownloadManager::createAndStartDownload(const QString &url, const QString &filePath, const QString& nameOfFile) {
    // 1. Створити DownloadItem
    DownloadItem *item = new DownloadItem(url, filePath, nullptr, nameOfFile);
    m_items.push_back(item);

    // 2. Створити та запустити завантаження
    startDownload(item);

    // 3. Сповістити UI
    emit downloadReadyToAdd(item);
}

void DownloadManager::startDownload(DownloadItem *item){
    DownloadTask *task = new DownloadTask(item->getUrl(), item->getFilePath());

    connect(item, &DownloadItem::statusChanged, task, &DownloadTask::setStatus, Qt::QueuedConnection);
    connect(task, &DownloadTask::progressChanged, item, &DownloadItem::onProgressChanged, Qt::QueuedConnection);
    connect(task, &DownloadTask::statusChanged, m_threadPool, &ThreadPool::chackWhatStatus, Qt::QueuedConnection);
    connect(task, &DownloadTask::statusChanged, item, &DownloadItem::chackWhatStatus, Qt::QueuedConnection);
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


/*void DownloadManager::resumeDownload(){
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    DownloadTask *task = m_itemTask[item];
    if(task){
        m_threadPool->resumeDownload(task);
    }
}*/

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
        DownloadTask *task = m_itemTask[item];
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
    QVector<DownloadRecord> records = m_db->getDownloadsInProcess();

    qDebug() << "downloads size: " << records.size();
    for(auto record : records){
        DownloadItem* item = new DownloadItem(record.m_name, record.m_directory);
        DownloadItemAdapter::updateItemFromRecord(item, record);
        //m_threadPool->addTask(item);
        //m_items.push_back(item);

        connect(item, &DownloadItem::deleteDownload, m_db, &DownloadDatabase::deleteDownload);

        emit setDownloadItemFromDB(item);
    }
}

void DownloadManager::saveAll(){
    for(auto item : m_items){
        m_db->saveDownload(DownloadItemAdapter::toRecord(*item));
    }
}

DownloadManager::~DownloadManager(){}
