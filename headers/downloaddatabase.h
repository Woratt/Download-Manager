#ifndef DOWNLOADDATABASE_H
#define DOWNLOADDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QMap>
#include <QList>
#include <QVariant>
#include <QRegularExpression>

#include "downloadrecord.h"

class DownloadDatabase : public QObject
{
    Q_OBJECT
public:
    explicit DownloadDatabase(const QString& dbPath = "", QObject *parent = nullptr);
    ~DownloadDatabase();
    bool initDatabase(const QString& dbPath = "");
    //int addDownload(const QString&, const QString&);
    //void updateDownloadProgress(int, int);
    //void updateDownloadStatus(int, const QString&);
    void saveDownloads(QVector<DownloadRecord> records);
    void deleteDownload(DownloadRecord record);
    QVector<DownloadRecord> getDownloads();
signals:
    void saveSuccesed();
private:
    QSqlDatabase m_db;
    bool createTables();
    bool isValidRecord(const DownloadRecord& record);
    void bindRecord(QSqlQuery& query, const DownloadRecord& record);
    bool isValidPath(const QString& pathToDataBase);
protected:
    QString getSystemDatabasePath();

    friend class DownloadDatabaseTest;
};

#endif // DOWNLOADDATABASE_H
