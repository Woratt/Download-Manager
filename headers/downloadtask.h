#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QObject>
#include <QRunnable>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QEventLoop>
#include <QThread>
#include <QTimer>
#include <memory>
#include <QFileInfo>
#include <QStorageInfo>
#include <QCryptographicHash>
#include <QRegularExpression>

#include "downloadrecord.h"
#include "chunkprocessor.h"
#include "networkmanager.h"
#include "storagemanager.h"

class DownloadTask :  public QObject
{
    Q_OBJECT
public:
    enum Status {
        Pending,
        Downloading,
        Resumed,
        StartNewTask,
        ResumedInPending,
        ResumedInDownloading,
        Paused,
        PausedNew,
        PausedResume,
        Completed,
        Error,
        Cancelled,
        Deleted
    };

    DownloadTask(const QString&, const QString&, qint64 fileSize, QObject *parent = nullptr);
    ~DownloadTask();
    Status getStatus(){
        qDebug() << "Status: " << m_status;
        return m_status;
    };
    void updateFromDb(const DownloadRecord &record);
signals:
    void progressChanged(qint64, qint64);
    void statusChanged(DownloadTask::Status);
    void paused();
    void stoped();
    void start();
    void checkFinished(bool isCorrupted);
    public slots:
    void startDownload();
    void pauseDownload();
    void resumeDownload();
    void stopDownload();
    void setStatus(Status);
    void saveChunckHash(int index, const QByteArray &data, const QByteArray &hash);
private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFinished();
    void onNetworkError(QNetworkReply::NetworkError);
private:
    QString m_url;
    QString m_filePath;
    QString m_remoteExpectedHash;
    QString m_actualHash;
    qint64 m_resumeDownloadPos;

    QStringList m_hashCandidates;
    QVector<QByteArray> m_chunkHashes;
    QCryptographicHash::Algorithm m_activeAlgorithm = QCryptographicHash::Sha256;

    void startHashDiscovery();
    void tryNextHashCandidate();
    void parseHashContent(const QByteArray &content, const QString &candidateUrl);

    QTimer *m_timeoutTimer;
    int m_timeoutSeconds{30};
    int m_currentTimeout;

    qint64 m_lastBytesReceived{0};
    QElapsedTimer m_speedTimer;

    double m_currentSpeed{0};
    QList<double> m_speedSamples;
    void measureSpeed(qint64);
    void adjustTimeout();

    int m_timeToRetry{2};
    int m_retryCount{0};
    const int MAX_RETRIES{5};

    bool m_isAborting{false};
    bool m_isHandlingError{false};


    Status m_status = Status::Pending;

    void handleFailure(const QString& errorContext, bool shouldRetry);
    bool isRetryableError(QNetworkReply::NetworkError);
    void onTimeout();
    void scheduleRetry(const QString&);
    void syncAndStop();

    bool m_isPauseRequested{false};

    void verifyHashOfFile();

    ChunkProcessor *m_chunkProcessor;
    NetworkManager *m_networkManager;
    StorageManager *m_storageManager;

    void setUpConnections();

    friend class DownloadAdapter;
};

#endif // DOWNLOADTASK_H
