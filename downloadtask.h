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

#include "downloadrecord.h"

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

    DownloadTask(const QString&, const QString&, QObject *parent = nullptr);
    ~DownloadTask();
    void startNewTask(QThread* thread);
    void stopDownload();
    Status getStatus(){
        qDebug() << "Status: " << m_status;
        return m_status;
    };
    void updateFromDb(const DownloadRecord &record);
signals:
    void progressChanged(qint64, qint64);
    void statusChanged(DownloadTask::Status);
    void paused();
public slots:
    void startDownload();
    void pauseDownload();
    void resumeDownload();
    void setStatus(Status);
private slots:
    void onReadyRead();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFinished();
    void onError(QNetworkReply::NetworkError);
private:
    QString m_url;
    QString m_filePath;
    QFile *m_outputFile;
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
    qint64 m_totalBytesWritten;
    qint64 m_resumeDownloadPos;

    std::unique_ptr<QByteArray> m_currentBuffer;
    std::unique_ptr<QByteArray> m_writeBuffer;
    bool m_isWriting{false};

    void createAndPrepareFile(qint64 totalBytes, QString &errorMessage);

    QTimer *m_timeoutTimer;
    int m_timeoutSeconds{30};
    int m_currentTimeout;

    qint64 m_lastBytesReceived = 0;
    QElapsedTimer m_speedTimer;
    qint64 m_bufferCapacity{64 * 1024};
    qint64 m_maxBufferCapacity{256 * 1024};
    qint64 m_minBufferCapacity{16 * 1024};
    double m_currentSpeed{0};
    QList<double> m_speedSamples;
    void measureSpeed(qint64);
    void adjustBufferSize();
    void adjustTimeout();

    int m_timeToRetry{2};
    int m_retryCount{0};
    const int MAX_RETRIES{5};

    bool m_isAborting{false};
    bool m_isHandlingError{false};

    void flushBufferAsync();

    Status m_status = Status::Pending;

    void handleFailure(const QString& errorContext, bool shouldRetry);
    bool isRetryableError(QNetworkReply::NetworkError);
    void onTimeout();
    void scheduleRetry(const QString&);
    void swapBuffers();

    friend class DownloadAdapter;
};

#endif // DOWNLOADTASK_H
