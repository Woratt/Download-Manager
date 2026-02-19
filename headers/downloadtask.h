#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QObject>
#include <QRunnable>
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
        FileIntegrityCheck,
        Preparing,
        Prepared,
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

    DownloadTask(const QString&, DownloadTypes::DownloadRecord fileInfo, QObject *parent = nullptr);
    ~DownloadTask();
    Status getStatus(){ return m_status; };
    DownloadTypes::DownloadRecord getFileInfo() const { return m_fileInfo; };
    void updateFromDb(const DownloadRecord &record);
signals:
    void progressChanged(qint64, qint64);
    void statusChanged(DownloadTask::Status);
    void paused();
    void stoped();
    void start();
    void deletedownloadedData(const DownloadTypes::DownloadRecord &fileInfo);
    void clearFile(const DownloadTypes::DownloadRecord &fileInfo);
    void openFile(const DownloadTypes::DownloadRecord &fileInfo, qint64 resumeDownloadPos);
    void stopWrite(const DownloadTypes::DownloadRecord &fileInfo);
    void checkFinished(bool isCorrupted);
    void writeChunk(const DownloadTypes::DownloadRecord &fileInfo, int index, const QByteArray &data);
public slots:
    void startDownload();
    void pauseDownload();
    void resumeDownload();
    void onFinished(const DownloadTypes::DownloadRecord &fileInfo);
    void stopDownload();
    void setStatus(Status);
    void saveAndWriteChunckHash(int index, const QByteArray &data, const QByteArray &hash);
    void changeQuantityOfChunks(const DownloadTypes::DownloadRecord &fileInfo, qint64 valueOfChange);
private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onNetworkError(QNetworkReply::NetworkError);
private:
    QString m_url;
    QString m_remoteExpectedHash;
    QString m_actualHash;
    qint64 m_resumeDownloadPos;

    DownloadTypes::DownloadRecord m_fileInfo;

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


    Status m_status = Status::Preparing;

    void handleFailure(const QString& errorContext, bool shouldRetry);
    bool isRetryableError(QNetworkReply::NetworkError);
    void onTimeout();
    void scheduleRetry(const QString&);
    void syncAndStop();

    bool m_isPauseRequested{false};

    void verifyHashOfFile();

    ChunkProcessor *m_chunkProcessor;
    NetworkManager *m_networkManager;

    void setUpConnections();

    friend class DownloadAdapter;
};

#endif // DOWNLOADTASK_H
