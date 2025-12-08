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

class DownloadTask :  public QObject
{
    Q_OBJECT
public:
    DownloadTask(const QString&, const QString&, QObject *parent = nullptr);
    ~DownloadTask();
    void startDownload();
    void startNewTask(QThread* thread);
    void resumeFromDB(QThread* thread, qint64);
    void stopDownload();
signals:
    void progressChanged(qint64, qint64);
    void finished(const QString&);
    void error(const QString&);
public slots:
    void pauseDownload();
    void resumeDownload();
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

    bool m_isPaused{false};
    bool m_isDownloading;
    bool m_isAborting{false};
    bool m_isHandlingError{false};

    void flushBufferAsync();

    void handleFailure(const QString& errorContext, bool shouldRetry);
    bool isRetryableError(QNetworkReply::NetworkError);
    void onTimeout();
    void scheduleRetry(const QString&);
    void swapBuffers();
};

#endif // DOWNLOADTASK_H
