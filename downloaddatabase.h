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

#include "downloadrecord.h"
#include "downloaditem.h"

class DownloadDatabase : public QObject
{
    Q_OBJECT
public:
    explicit DownloadDatabase(QObject *parent = nullptr);
    bool initDatabase();
    int addDownload(const QString&, const QString&);
    void updateDownloadProgress(int, int);
    void updateDownloadStatus(int, const QString&);
    void saveDownload(const DownloadRecord&);
    void deleteDownload(DownloadItem*);
    QVector<DownloadRecord> getDownloadsInProcess();
private:
    QSqlDatabase m_db;
    QString getDatabasePath();
    bool createTables();
};

#endif // DOWNLOADDATABASE_H
