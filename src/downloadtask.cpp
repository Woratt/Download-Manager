#include "../headers/downloadtask.h"

DownloadTask::DownloadTask(const QString& url, DownloadTypes::FileInfo fileInfo, QObject *parent) :
                                                                    QObject(parent),
                                                                    m_url(url),
                                                                    m_fileInfo(fileInfo),
                                                                    m_resumeDownloadPos(0)
{
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);

    m_chunkProcessor = new ChunkProcessor(this);
    m_networkManager = new NetworkManager(this);

    setUpConnections();
}

void DownloadTask::setUpConnections(){
    connect(m_networkManager, &NetworkManager::dataReceived, m_chunkProcessor, &ChunkProcessor::processData);
    connect(m_networkManager, &NetworkManager::downloadProgress, this, &DownloadTask::onDownloadProgress);
    connect(m_networkManager, &NetworkManager::errorOccurred, this, &DownloadTask::onNetworkError);
    connect(m_networkManager, &NetworkManager::finished, m_chunkProcessor, &ChunkProcessor::finalize);
    connect(m_chunkProcessor, &ChunkProcessor::chunkReady, this, &DownloadTask::saveAndWriteChunckHash);

    connect(m_timeoutTimer, &QTimer::timeout, this, &DownloadTask::onTimeout);
}

void DownloadTask::updateFromDb(const DownloadRecord &record){
    m_resumeDownloadPos = record.m_downloadedBytes;

    m_remoteExpectedHash = record.m_expectedHash;
    m_actualHash = record.m_actualHash;
    if(record.m_hashAlgorithm == "md5"){
        //m_activeAlgorithm = QCryptographicHash::Md5;
       // m_chunkProcessor->setCryptographicAlgorithm(m_activeAlgorithm);
    } else {
        m_activeAlgorithm = QCryptographicHash::Sha256;
        //m_chunkProcessor->setCryptographicAlgorithm(m_activeAlgorithm);
    }
    QByteArray chunkData = record.m_chunkHashes;
    QDataStream in(&chunkData, QIODevice::ReadOnly);
    in >> m_chunkHashes;

    QString status = record.m_status;
    if (record.m_status == "pending") m_status = DownloadTask::Pending;
    if (record.m_status == "downloading") m_status = DownloadTask::Downloading;
    if (record.m_status == "resumed") m_status = DownloadTask::Resumed;
    if (record.m_status == "start_new_task") m_status = DownloadTask::StartNewTask;
    if (record.m_status == "resumed_in_pending") m_status = DownloadTask::ResumedInPending;
    if (record.m_status == "resumed_in_downloading") m_status = DownloadTask::ResumedInDownloading;
    if (record.m_status == "paused") m_status = DownloadTask::Paused;
    if (record.m_status == "paused_new") m_status = DownloadTask::PausedNew;
    if (record.m_status == "paused_resume") m_status = DownloadTask::PausedResume;
    if (record.m_status == "completed") m_status = DownloadTask::Completed;
    if (record.m_status == "error") m_status = DownloadTask::Error;
    if (record.m_status == "cancelled") m_status = DownloadTask::Cancelled;
    if (record.m_status == "deleted") m_status = DownloadTask::Deleted;
}

void DownloadTask::saveAndWriteChunckHash(int index, const QByteArray &data, const QByteArray &hash){
    m_timeoutTimer->start(m_timeoutSeconds * 1000);
    if(!hash.isEmpty()){
        m_chunkHashes.append(hash);

        emit writeChunk(m_fileInfo, index, data);
    }
}

void DownloadTask::setStatus(Status newStatus){
    if(newStatus != m_status){
        m_status = newStatus;
        emit statusChanged(m_status);
    }
}

void DownloadTask::startDownload(){
    setStatus(Status::Downloading);
    startHashDiscovery();

    connect(this, &DownloadTask::start, this, [=](){
        m_networkManager->startDownload(m_url);
    }, Qt::SingleShotConnection);
}

void DownloadTask::startHashDiscovery()
{
    m_hashCandidates.clear();
    QUrl url(m_url);
    QString baseUrl = m_url.left(m_url.lastIndexOf('/') + 1);
    QString fileName = url.fileName();

    qDebug() << "baseUrl: " << baseUrl;

    m_hashCandidates.append(m_url + ".sha256");
    m_hashCandidates.append(m_url + ".sha1");
    m_hashCandidates.append(m_url + ".md5");
    m_hashCandidates.append(baseUrl + "SHA256SUMS");
    m_hashCandidates.append(baseUrl + "sha256sums.txt");
    m_hashCandidates.append(baseUrl + "MD5SUMS");
    m_hashCandidates.append(baseUrl + "md5sums.txt");
    m_hashCandidates.append(baseUrl + "checksums.txt");

    qDebug() << "Search for a reference hash for:" << fileName;
    tryNextHashCandidate();
}

void DownloadTask::tryNextHashCandidate()
{
    if (m_hashCandidates.isEmpty()) {
        qDebug() << "Reference not found. Starting download without verification.";
        emit start();
        return;
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString candidate = m_hashCandidates.takeFirst();
    qDebug() << "Str: " << candidate;
    QNetworkRequest request((QUrl(candidate)));

    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, candidate, manager]() {
        if (reply->error() == QNetworkReply::NoError) {
            parseHashContent(reply->readAll(), candidate);
            reply->deleteLater();

            if (!m_remoteExpectedHash.isEmpty()) {
                qDebug() << "Reference obtained! Starting download...";
                emit start();
            } else {
                tryNextHashCandidate();
            }
        } else {
            reply->deleteLater();
            tryNextHashCandidate();
        }
        manager->deleteLater();
    });
}

void DownloadTask::parseHashContent(const QByteArray &content, const QString &candidateUrl)
{
    QString data = QString::fromUtf8(content).trimmed();
    QString fileName = QUrl(m_url).fileName();

    if (candidateUrl.toLower().contains("md5")) {
        //m_activeAlgorithm = QCryptographicHash::Md5;
        //m_chunkProcessor->setCryptographicAlgorithm(m_activeAlgorithm);
    } else {
        //m_activeAlgorithm = QCryptographicHash::Sha256;
        //m_chunkProcessor->setCryptographicAlgorithm(m_activeAlgorithm);
    }

    QStringList lines = data.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.contains(fileName, Qt::CaseInsensitive)) {
            QRegularExpression hashRegex("([a-fA-F0-9]{32,128})");
            QRegularExpressionMatch match = hashRegex.match(line);

            if (match.hasMatch()) {
                m_remoteExpectedHash = match.captured(1).toUtf8().toLower();
                qDebug() << "🎯 Знайдено хеш (" << candidateUrl.section('/', -1) << "):" << m_remoteExpectedHash;
                return;
            }
        }
    }
}

void DownloadTask::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal){
    //m_timeoutTimer->start(m_timeoutSeconds * 1000);

    measureSpeed(bytesReceived + m_resumeDownloadPos);

    m_timeToRetry = 1;

    emit progressChanged(bytesReceived + m_resumeDownloadPos, bytesTotal + m_resumeDownloadPos);
}

void DownloadTask::measureSpeed(qint64 bytesReceived){
    if(!m_speedTimer.isValid()){
        m_speedTimer.start();
        m_lastBytesReceived = bytesReceived;
        return;
    }

    qint64 elapsed = m_speedTimer.elapsed();
    if(elapsed > 1000){
        qint64 bytesDiff = bytesReceived - m_lastBytesReceived;
        double instantSpeed = (bytesDiff * 1000.0) / elapsed;

        m_speedSamples.append(instantSpeed);
        if (m_speedSamples.size() > 5) {
            m_speedSamples.removeFirst();
        }

        m_currentSpeed = std::accumulate(m_speedSamples.begin(), m_speedSamples.end(), 0.0) / m_speedSamples.size();
        m_lastBytesReceived = bytesReceived;
        m_speedTimer.restart();

        adjustTimeout();
    }
}

void DownloadTask::adjustTimeout() {
    if (m_currentSpeed <= 0) return;

    int newTimeout = m_timeoutSeconds;

    if (m_currentSpeed < 1 * 1024 * 1024) {
        newTimeout = 60;
    }
    else if (m_currentSpeed < 3 * 1024 * 1024) {
        newTimeout = 45;
    }
    else {
        newTimeout = 30;
    }

    if (newTimeout != m_currentTimeout) {
        m_currentTimeout = newTimeout;
        m_timeoutSeconds = newTimeout;

        if (m_timeoutTimer->isActive()) {
            m_timeoutTimer->start(m_timeoutSeconds * 1000);
        }
    }
}

void DownloadTask::onFinished(const DownloadTypes::FileInfo &fileInfo){
    if(fileInfo == m_fileInfo){

        m_networkManager->abort();
        emit stopWrite(m_fileInfo);

        m_timeoutTimer->stop();

        if(!m_remoteExpectedHash.isEmpty()){
            QFile file(m_fileInfo.filePath);
            if (!file.open(QIODevice::ReadOnly)) return;

            QCryptographicHash hasher(m_activeAlgorithm);
            while (!file.atEnd()) {
                hasher.addData(file.read(1024 * 1024));
            }

            QString localHash = hasher.result().toHex().toLower();

            qDebug() << "localHash: " << localHash;
            qDebug() << "m_remoteExpectedHash: " << m_remoteExpectedHash;

            bool isOk = (localHash == m_remoteExpectedHash);

            //qDebug() << (isOk ? "✅ Файл валідний" : "❌ Файл пошкоджено!");
        }else{
            verifyHashOfFile();

            connect(this, &DownloadTask::checkFinished, this, [=](bool isCorrupted){
                //qDebug() << (!isCorrupted ? "✅ Файл валідний" : "❌ Файл пошкоджено!");
            }, Qt::SingleShotConnection);
        }

        setStatus(Status::Completed);
    }
}

void DownloadTask::stopDownload(){
    syncAndStop();

    emit stoped();
}

void DownloadTask::pauseDownload(){
    //setStatus(Status::Paused);
    syncAndStop();

    emit paused();
}

void DownloadTask::resumeDownload(){
    //QThread* currentObjThread = this->thread();
    //QThread* currentExecThread = QThread::currentThread();

    //qDebug() << "The object belongs to the thread:" << currentObjThread;
    //qDebug() << "The code is now running in the thread:" << currentExecThread;

    connect(this, &DownloadTask::checkFinished, this, [=](bool isCorrupted){
        if(isCorrupted){
            m_resumeDownloadPos = 0;

            syncAndStop();

            m_chunkHashes.clear();
            emit clearFile(m_fileInfo);
            qDebug() << "- The existing chunks have been checked. File corrupted";
        }else{
            qDebug() << "+ The existing chunks have been checked. Let's continue...";
        }

        m_chunkProcessor->reset(m_resumeDownloadPos / (1024 * 1024));
        m_networkManager->startDownload(m_url, m_resumeDownloadPos);
        emit openFile(m_fileInfo, m_resumeDownloadPos);
    }, Qt::SingleShotConnection);

    verifyHashOfFile();
}

bool DownloadTask::isRetryableError(QNetworkReply::NetworkError error){
    return (error == QNetworkReply::ConnectionRefusedError ||
            error == QNetworkReply::RemoteHostClosedError ||
            error == QNetworkReply::TimeoutError ||
            error == QNetworkReply::TemporaryNetworkFailureError ||
            error == QNetworkReply::UnknownNetworkError ||
            error == QNetworkReply::HostNotFoundError) ||
            error == QNetworkReply::OperationCanceledError ||
            error == QNetworkReply::ServiceUnavailableError ||
            error == QNetworkReply::UnknownServerError ||
            error == QNetworkReply::InternalServerError;
}

void DownloadTask::onNetworkError(QNetworkReply::NetworkError errorCode){
    if (m_status == Status::Paused || m_status == Status::PausedResume
        || m_status == Status::PausedNew || m_status == Status::Pending || m_isHandlingError) return;
    m_isHandlingError = true;

    syncAndStop();

    qDebug() << "1";

    bool shouldRetry = isRetryableError(errorCode);

    handleFailure("Network error: ", shouldRetry);
    m_isHandlingError = false;
}

void DownloadTask::handleFailure(const QString& errorContext, bool shouldRetry){
    if(m_status == Status::Paused) return;

    if(shouldRetry && m_retryCount < MAX_RETRIES){
        scheduleRetry(errorContext);
    }else{
        setStatus(Status::Error);
        m_retryCount = 0;
    }
}

void DownloadTask::onTimeout(){
    m_isHandlingError = true;
    if (m_status == Status::Paused || m_status == Status::PausedResume
        || m_status == Status::PausedNew || m_status == Status::Pending || m_isHandlingError) return;

    syncAndStop();

    qDebug() << "Download timeout after" << m_timeoutSeconds << "seconds";

    handleFailure("Timeout", true);
    m_isHandlingError = false;
}

void DownloadTask::scheduleRetry(const QString& error){
    ++m_retryCount;

    QTimer::singleShot(m_timeToRetry * 1000, this, &DownloadTask::resumeDownload);
    m_timeToRetry <<= 1;
}

void DownloadTask::syncAndStop() {
    m_networkManager->abort();
    m_resumeDownloadPos = static_cast<qint64>(m_chunkProcessor->getCurrentIndex()) * 1024 * 1024;
    m_chunkProcessor->reset(m_chunkProcessor->getCurrentIndex());
    emit stopWrite(m_fileInfo);
}

void DownloadTask::verifyHashOfFile(){
    if (m_chunkHashes.isEmpty() || m_resumeDownloadPos == 0) emit checkFinished(false);

    QFile file(m_fileInfo.filePath);
    if (!file.open(QIODevice::ReadOnly)) emit checkFinished(true);

    for (int i = 0; i < m_chunkHashes.size(); ++i) {
        QByteArray chunkData = file.read(1024 * 1024);
        QByteArray currentHash = QCryptographicHash::hash(chunkData, m_activeAlgorithm).toHex();

        if (currentHash != m_chunkHashes[i]) {
            emit checkFinished(true);
        }
    }
    emit checkFinished(false);
}

void DownloadTask::changeQuantityOfChunks(const DownloadTypes::FileInfo &fileInfo, qint64 valueOfChange){
    if(m_fileInfo == fileInfo){
        m_fileInfo.quantityOfChunks += valueOfChange;
    }
}

DownloadTask::~DownloadTask(){
    syncAndStop();
    emit deletedownloadedData(m_fileInfo);
}
