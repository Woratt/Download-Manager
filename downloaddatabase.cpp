#include "downloaddatabase.h"

DownloadDatabase::DownloadDatabase(QObject *parent) : QObject(parent) {
    initDatabase();
}

QString DownloadDatabase::getDatabasePath(){

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir dir(dataPath);
    if(!dir.exists()){
        dir.mkpath(".");
    }
    qDebug() << dir.filePath("download.db");

    return dir.filePath("download.db");
}

bool DownloadDatabase::initDatabase(){
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(getDatabasePath());

    if(!m_db.open()){
        return false;
    }

    return createTables();
}

bool DownloadDatabase::createTables(){
    QSqlQuery query(m_db);

    QString createDownloadTables =
        "CREATE TABLE IF NOT EXISTS downloads ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "url TEXT NOT NULL UNIQUE,"
        "directory TEXT,"
        "status TEXT NOT NULL DEFAULT 'pending',"
        "totalBytes INTEGER DEFAULT 0,"
        "downloadedBytes INTEGER DEFAULT 0,"
        "resumePosition INTEGER DEFAULT 0,"
        "currentSpeed INTEGER DEFAULT 0,"
        "isChecked BOOLEAN DEFAULT 0,"
        "errorMessage TEXT,"
        "createdAt DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updatedAt DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")";

    if(!query.exec(createDownloadTables)){
        qDebug() << "Error creating downloads table:" << query.lastError().text();
        return false;
    }

    QString createQueueTable =
        "CREATE TABLE IF NOT EXISTS downloads_queue ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "download_id INTEGER, "
        "priority INTEGER DEFAULT 0, "
        "FOREIGN KEY (download_id) REFERENCES downloads (id)"
        ")";

    if(!query.exec(createQueueTable)){
        qDebug() << "Error creating queue table:" << query.lastError().text();
        return false;
    }

    return true;
}

int DownloadDatabase::addDownload(const QString& url, const QString& filePath){
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO downloads (url, file_path, status) VALUES (?, ?, 'pending')");
    query.addBindValue(url);
    query.addBindValue(filePath);

    if(!query.exec()){
        qDebug() << "Error adding download:" << query.lastError().text();
        return -1;
    }

    return query.lastInsertId().toInt();
}

void DownloadDatabase::updateDownloadProgress(int id, int progress)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE downloads SET progress = ? WHERE id = ?");
    query.addBindValue(progress);
    query.addBindValue(id);

    if(!query.exec()){
        qDebug() << "Error updating progress:" << query.lastError().text();
    }
}

void DownloadDatabase::updateDownloadStatus(int id, const QString& status)
{
    QSqlQuery query(m_db);

    if(status == "completed"){
        query.prepare("UPDATE downloads SET status = ?, completed_at = CURRENT_TIMESTAMP WHERE id = ?");
    }else{
        query.prepare("UPDATE downloads SET status = ? WHERE id = ?");
    }

    query.addBindValue(status);
    query.addBindValue(id);
    if(!query.exec()){
        qDebug() << "Error updating status:" << query.lastError().text();
    }

}

QVector<DownloadRecord> DownloadDatabase::getDownloadsInProcess()
{
    QVector<DownloadRecord> records;

    QSqlQuery query(m_db);

    query.exec("SELECT * FROM downloads");

    while(query.next()){
        DownloadRecord record;

        record.m_name = query.value("name").toString();
        record.m_url = query.value("url").toString();
        record.m_directory = query.value("directory").toString();
        record.m_status = query.value("status").toString();
        record.m_totalBytes = query.value("totalBytes").toLongLong();
        record.m_downloadedBytes = query.value("downloadedBytes").toLongLong();
        record.m_resumePosition = query.value("resumePosition").toLongLong();
        record.m_currentSpeed = query.value("currentSpeed").toLongLong();
        record.m_isChecked = query.value("isChecked").toBool();
        record.m_errorMessage = query.value("errorMessage").toString();
        record.m_createdAt = query.value("createdAt").toDateTime();
        record.m_updatedAt = query.value("updatedAt").toDateTime();

        records.push_back(record);

    }

    return records;
}

void DownloadDatabase::saveDownload(const DownloadRecord& record){
    QSqlQuery query(m_db);

    QString request =
    "INSERT OR REPLACE INTO downloads "
    "(name, url, directory, status, totalBytes, downloadedBytes, "
    "resumePosition, currentSpeed, isChecked, errorMessage, updatedAt) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    query.prepare(request);

    query.addBindValue(record.m_name);
    query.addBindValue(record.m_url);
    query.addBindValue(record.m_directory);
    query.addBindValue(record.m_status);
    query.addBindValue(record.m_totalBytes);
    query.addBindValue(record.m_downloadedBytes);
    query.addBindValue(record.m_resumePosition);
    query.addBindValue(record.m_currentSpeed);
    query.addBindValue(record.m_isChecked);
    query.addBindValue(record.m_errorMessage);
    query.addBindValue(record.m_updatedAt);

    if(!query.exec()){
        qDebug() << "Error save download " << query.lastError().text();
    }
}

void DownloadDatabase::deleteDownload(DownloadItem* item){
    QSqlQuery query(m_db);

    query.prepare("DELETE FROM downloads WHERE url = ?");
    query.addBindValue(item->getUrl());

    if(!query.exec()){
        qDebug() << "Error delete download " << query.lastError().text();
    }

    qDebug() << "deleteDownload";
}


