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

    QString createTableSql =
        "CREATE TABLE IF NOT EXISTS downloads ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "url TEXT NOT NULL UNIQUE,"
        "filePath TEXT,"
        "status TEXT NOT NULL DEFAULT 'pending',"
        "totalBytes INTEGER DEFAULT 0,"
        "downloadedBytes INTEGER DEFAULT 0,"
        "createdAt DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updatedAt DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")";

    if(!query.exec(createTableSql)){
        qDebug() << "Error creating downloads table:" << query.lastError().text();
        return false;
    }
    query.exec("CREATE INDEX IF NOT EXISTS idx_status ON downloads(status)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_url ON downloads(url)");

    if(!query.exec(createTableSql)){
        qDebug() << "Error creating downloads table:" << query.lastError().text();
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

QVector<DownloadRecord> DownloadDatabase::getDownloads()
{
    QVector<DownloadRecord> records;

    QSqlQuery query(m_db);

    query.exec("SELECT name, url, filePath, status, totalBytes, downloadedBytes FROM downloads "
               "ORDER BY "
               "CASE status "
               "WHEN 'downloading' THEN 1 "
               "WHEN 'resumed' THEN 2 "
               "WHEN 'resumed_in_downloading' THEN 3 "
               "WHEN 'start_new_task' THEN 4 "
               "WHEN 'resumed_in_pending' THEN 5 "
               "WHEN 'pending' THEN 6 "
               "WHEN 'paused' THEN 7 "
               "WHEN 'paused_new' THEN 8 "
               "WHEN 'paused_resume' THEN 9 "
               "WHEN 'completed' THEN 10 "
               "WHEN 'error' THEN 11 "
               "WHEN 'cancelled' THEN 12 "
               "WHEN 'deleted' THEN 13 "
               "END ASC;");
    while(query.next()){
        DownloadRecord record;

        record.m_name = query.value(0).toString();
        record.m_url = query.value(1).toString();
        record.m_filePath = query.value(2).toString();
        record.m_status = query.value(3).toString();
        record.m_totalBytes = query.value(4).toInt();
        record.m_downloadedBytes = query.value(5).toLongLong();

        records.push_back(record);
    }
    return records;
}


void DownloadDatabase::saveDownload(const DownloadRecord& record){
    QSqlQuery query(m_db);

    QString request =
    "INSERT OR REPLACE INTO downloads "
    "(name, url, filePath, status, totalBytes, downloadedBytes) "
    "VALUES (?, ?, ?, ?, ?, ?)";

    query.prepare(request);

    query.addBindValue(record.m_name);
    query.addBindValue(record.m_url);
    query.addBindValue(record.m_filePath);
    query.addBindValue(record.m_status);
    query.addBindValue(record.m_totalBytes);
    query.addBindValue(record.m_downloadedBytes);

    if(!query.exec()){
        qDebug() << "Error save download " << query.lastError().text();
    }
}

void DownloadDatabase::deleteDownload(DownloadItem* item){
    QSqlQuery query(m_db);

    query.prepare("DELETE FROM downloads WHERE name = ? AND url = ? AND filePath = ?");
    query.addBindValue(item->getName());
    query.addBindValue(item->getUrl());
    query.addBindValue(item->getFilePath());

    if(!query.exec()){
        qDebug() << "Error delete download " << query.lastError().text();
    }

    qDebug() << "deleteDownload";
}


