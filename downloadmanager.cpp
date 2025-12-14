#include "downloadmanager.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent){
    m_threadPool = new ThreadPool(this);
    m_db = new DownloadDatabase(this);
}

void DownloadManager::startDownload(DownloadItem *item){
    DownloadTask *task = new DownloadTask(item->getUrl(), item->getFilePath());

    connect(item, &DownloadItem::statusChanged, task, &DownloadTask::setStatus, Qt::QueuedConnection);
    connect(task, &DownloadTask::progressChanged, item, &DownloadItem::onProgressChanged, Qt::QueuedConnection);
    connect(task, &DownloadTask::statusChanged, m_threadPool, &ThreadPool::chackWhatStatus, Qt::QueuedConnection);
    connect(task, &DownloadTask::statusChanged, item, &DownloadItem::chackWhatStatus, Qt::QueuedConnection);

    connect(item, &DownloadItem::ChangedBt, this, &DownloadManager::changeBt);

    m_threadPool->addTask(task);

    m_itemTask[item] = task;
}

void DownloadManager::pausedDownload(const QString& url){

}

void DownloadManager::resumeDownload(){
    DownloadItem *item = qobject_cast<DownloadItem*>(sender());
    DownloadTask *task = m_itemTask[item];
    if(task){
        m_threadPool->resumeDownload(task);
    }
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
