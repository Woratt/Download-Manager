#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFileInfo>

struct RemoteFileInfo {
    QUrl url;
    qint64 fileSize = 0;
    QString fileName;
    bool supportsRange = false;
    QString mimeType;
    bool isValid = false;
    QString errorString;
    QString suffix;
};


class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);
    void getRemoteFileInfo(const QUrl &url);
    void abort();
public slots:
    void startDownload(const QUrl &url, qint64 startByte = 0);
private:
    QNetworkRequest prepareRequest(const QUrl &url, qint64 startByte = 0);
    QString parseFileName(QNetworkReply *reply);
signals:
    void fileInfoReady(RemoteFileInfo fileInfo);
    void dataReceived(const QByteArray &data);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void errorOccurred(QNetworkReply::NetworkError error);
    void finished();
private slots:
    void onReadyRead();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFinished();
    void onError(QNetworkReply::NetworkError);
private:
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
};


#endif // NETWORKMANAGER_H
