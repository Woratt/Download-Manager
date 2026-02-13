#include "../headers/downloaddatabase.h"

DownloadDatabase::DownloadDatabase(const QString& dbPath, QObject *parent) : QObject(parent)
{
    if(!initDatabase(dbPath)){
        qDebug() << "Failed to initialize database";
    }
}

QString DownloadDatabase::getSystemDatabasePath(){

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir dir(dataPath);
    if(!dir.exists()){
        dir.mkpath(".");
    }

    return dir.filePath("download.db");
}

bool DownloadDatabase::initDatabase(const QString& dbPath){
    if(!dbPath.isEmpty() && !isValidPath(dbPath)){
        return false;
    }

    QString finalPath = dbPath.isEmpty() ? getSystemDatabasePath() : dbPath;

    QFileInfo fileInfo(finalPath);
    QDir parentDir = fileInfo.absoluteDir();

    if (!parentDir.exists()) {
        if (!parentDir.mkpath(".")) {
            qCritical() << "Could not create database directory:" << parentDir.absolutePath();
            return false;
        }
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(finalPath);

    if (!m_db.open()) {
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
        "expectedHash TEXT,"
        "actualHash TEXT,"
        "hashAlgorithm TEXT,"
        "chunkHashes BLOB,"
        "createdAt DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updatedAt DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")";

    query.exec("CREATE INDEX IF NOT EXISTS idx_status ON downloads(status)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_url ON downloads(url)");

    if(!query.exec(createTableSql)){
        qDebug() << "Error creating downloads table:" << query.lastError().text();
        return false;
    }

    return true;
}

/*int DownloadDatabase::addDownload(const QString& url, const QString& filePath){
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO downloads (url, filePath, status) VALUES (?, ?, 'pending')");
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
}*/

/*void DownloadDatabase::updateDownloadStatus(int id, const QString& status)
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

}*/

QVector<DownloadRecord> DownloadDatabase::getDownloads()
{
    QVector<DownloadRecord> records;

    QSqlQuery query(m_db);

    query.exec("SELECT name, url, filePath, status, totalBytes, downloadedBytes, expectedHash, actualHash, hashAlgorithm, chunkHashes FROM downloads "
               "ORDER BY "
               "CASE status "
               "WHEN 'downloading' THEN 1 "
               "WHEN 'resumed' THEN 2 "
               "WHEN 'resumed_in_downloading' THEN 3 "
               "WHEN 'start_new_task' THEN 4 "
               "WHEN 'resumed_in_pending' THEN 5 "
               "WHEN 'pending' THEN 6 "
               "WHEN 'prepared' THEN 7 "
               "WHEN 'preparing' THEN 8 "
               "WHEN 'paused' THEN 9 "
               "WHEN 'paused_new' THEN 10 "
               "WHEN 'paused_resume' THEN 11 "
               "WHEN 'completed' THEN 12 "
               "WHEN 'error' THEN 13 "
               "WHEN 'cancelled' THEN 14 "
               "WHEN 'deleted' THEN 15 "
               "END ASC;");
    while(query.next()){
        DownloadRecord record;

        record.m_name = query.value(0).toString();
        record.m_url = query.value(1).toString();
        record.m_filePath = query.value(2).toString();
        record.m_status = query.value(3).toString();
        record.m_totalBytes = query.value(4).toInt();
        record.m_downloadedBytes = query.value(5).toLongLong();
        record.m_expectedHash = query.value(6).toString();
        record.m_actualHash = query.value(7).toString();
        record.m_hashAlgorithm = query.value(8).toString();
        record.m_chunkHashes = query.value(9).toByteArray();

        records.push_back(record);
    }
    return records;
}

bool DownloadDatabase::isValidRecord(const DownloadRecord &record)
{
    if(record.m_name.isEmpty() ||
        record.m_url.isEmpty() ||
        record.m_filePath.isEmpty() ||
        record.m_status.isEmpty() ||
        record.m_totalBytes < 0
        ) return false;

    return true;
}

void DownloadDatabase::bindRecord(QSqlQuery& query, const DownloadRecord& record)
{
    query.addBindValue(record.m_name);
    query.addBindValue(record.m_url);
    query.addBindValue(record.m_filePath);
    query.addBindValue(record.m_status);
    query.addBindValue(record.m_totalBytes);
    query.addBindValue(record.m_downloadedBytes);
    query.addBindValue(record.m_expectedHash);
    query.addBindValue(record.m_actualHash);
    query.addBindValue(record.m_hashAlgorithm);
    query.addBindValue(record.m_chunkHashes, QSql::In | QSql::Binary);
}


void DownloadDatabase::saveDownloads(QVector<DownloadRecord> records){
    if (records.isEmpty()) return;

    QVector<DownloadRecord> validRecords;
    for (const auto& record : records) {
        if (isValidRecord(record)) {
            validRecords.append(record);
        } else {
            qDebug() << "Validation failed for record, skipping:" << record.m_url;
        }
    }

    if (validRecords.isEmpty()) return;

    QString request =
        "INSERT OR REPLACE INTO downloads "
        "(name, url, filePath, status, totalBytes, downloadedBytes, expectedHash, actualHash, hashAlgorithm, chunkHashes) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    if (m_db.transaction()) {
        QSqlQuery query(m_db);
        query.prepare(request);

        bool allOk = true;
        for (const auto& record : validRecords) {
            bindRecord(query, record);
            if (!query.exec()) {
                allOk = false;
                break;
            }
        }

        if (allOk && m_db.commit()) {
            emit saveSuccesed();
            return;
        }

        m_db.rollback();
        qDebug() << "Batch save failed, falling back to individual mode...";
    }

    QSqlQuery query(m_db);
    query.prepare(request);

    for (const auto& record : validRecords) {
        bindRecord(query, record);
        if (!query.exec()) {
            qDebug() << "Individual save failed for:" << record.m_url << query.lastError().text();
        }
    }

    emit saveSuccesed();
}

bool DownloadDatabase::isValidPath(const QString& path)
{
    if(path == ":memory:") return true;
    if (path.isEmpty()) return false;

    QFileInfo fileInfo(path);
    QString fileName = fileInfo.fileName();

#ifdef Q_OS_WIN
    static QRegularExpression winInvalidChars("[<>:\"|?*\\\\/]");
    if (fileName.contains(winInvalidChars)) {
        qWarning() << "Windows: Illegal characters in filename";
        return false;
    }
    static QRegularExpression winReservedNames("^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(\\..*)?$", QRegularExpression::CaseInsensitiveOption);
    if (fileName.contains(winReservedNames)) {
        qWarning() << "Windows: Reserved filename used";
        return false;
    }
#elif defined(Q_OS_MAC)
    static QRegularExpression macInvalidChars("[:/]");
    if (fileName.contains(macInvalidChars)) return false;
#else
    if (fileName.contains('/')) return false;
#endif

    if(fileInfo.exists() && fileInfo.isDir() ){
        return false;
    }

    if(fileName.isEmpty()){
        return false;
    }

    QDir parentDir = fileInfo.absoluteDir();
    if(!parentDir.exists()){
        QString checkPath = parentDir.absolutePath();

        while(!QDir(checkPath).exists() && checkPath.length() > 1){
            checkPath = QFileInfo(checkPath).absolutePath();
        }

        QFileInfo ancestorInfo(checkPath);
        if(!ancestorInfo.isWritable()){
            qWarning() << "Validation failed: No write permissions for" << checkPath;
            return false;
        }
    }else{
        QFileInfo dirInfo(parentDir.absolutePath());
        if (!dirInfo.isWritable()) {
            qWarning() << "Validation failed: Directory is read-only ->" << parentDir.absolutePath();
            return false;
        }
    }

    return true;
}

void DownloadDatabase::deleteDownload(DownloadRecord record){
    QSqlQuery query(m_db);

    query.prepare("DELETE FROM downloads WHERE url = ?");
    query.addBindValue(record.m_url);

    if(!query.exec()){
        qDebug() << "Error delete download " << query.lastError().text();
    }

    qDebug() << "deleteDownload";
}

DownloadDatabase::~DownloadDatabase(){
    if (m_db.isOpen()) m_db.close();
}


