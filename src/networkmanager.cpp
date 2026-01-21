#include "../headers/networkmanager.h"

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
    m_manager = new QNetworkAccessManager(this);
}

QNetworkRequest NetworkManager::prepareRequest(const QUrl &url, qint64 startByte){
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

    QString origin = url.scheme() + "://" + url.host() + "/";
    request.setRawHeader("Referer", origin.toUtf8());

    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "uk-UA,uk;q=0.9,en-US;q=0.8,en;q=0.7");

    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Upgrade-Insecure-Requests", "1");

    if(startByte > 0){
        QString range = QString("bytes=%1-").arg(startByte);
        request.setRawHeader("Range", range.toUtf8());
    }

    return request;
}


void NetworkManager::getRemoteFileInfo(const QUrl& url){
    QNetworkRequest request = prepareRequest(url);

    QNetworkReply *reply = m_manager->head(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        RemoteFileInfo info;
        info.url = url;

        if (reply->error() == QNetworkReply::NoError) {
            info.isValid = true;
            info.fileSize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
            info.supportsRange = (reply->rawHeader("Accept-Ranges") == "bytes");
            info.mimeType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
            info.fileName = parseFileName(reply);

            QFileInfo fileDetails(info.fileName);
            info.suffix = "." + fileDetails.suffix();
        } else {
            info.isValid = false;
            info.errorString = reply->errorString();
        }

        emit fileInfoReady(info);
        reply->deleteLater();
    });
}

QString NetworkManager::parseFileName(QNetworkReply *reply){
    QString disposition = reply->header(QNetworkRequest::ContentDispositionHeader).toString();
    QString name;

    if(disposition.contains("filename=")){
        name = disposition.section("filename=", 1).section(';', 0).trimmed();
        name.remove('\"');
    }else{
        name = reply->url().fileName();
    }

    return name;
}


void NetworkManager::startDownload(const QUrl &url, qint64 startByte){
    m_reply = m_manager->get(prepareRequest(url, startByte));

    connect(m_reply, &QNetworkReply::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &NetworkManager::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &NetworkManager::onFinished);
    connect(m_reply, &QNetworkReply::errorOccurred, this, &NetworkManager::onError);
}

void NetworkManager::onReadyRead(){
    if (m_reply) {
        QByteArray data = m_reply->readAll();
        if (!data.isEmpty()) {
            emit dataReceived(data);
        }
    }
}

void NetworkManager::abort(){
    if (m_reply && m_reply->isRunning()) {
        m_reply->abort();
    }
}

void NetworkManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal){
    emit downloadProgress(bytesReceived, bytesTotal);
}

void NetworkManager::onFinished() {
    if (m_reply) {
        if (m_reply->error() == QNetworkReply::NoError) {
            emit finished();
        }
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

void NetworkManager::onError(QNetworkReply::NetworkError code) {
    if (m_reply) {
        emit errorOccurred(code);
    }
}

