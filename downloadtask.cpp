#include "downloadtask.h"

DownloadTask::DownloadTask(const QString& url, const QString& filePath, QObject *parent) :
                                                                    QObject(parent),
                                                                    m_url(url),
                                                                    m_filePath(filePath),
                                                                    m_bufferCapacity(64 * 1024),
                                                                    m_totalBytesWritten(0),
                                                                    m_resumeDownloadPos(0)
{
    m_manager = new QNetworkAccessManager(this);
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);

    m_currentBuffer = std::make_unique<QByteArray>();
    m_writeBuffer = std::make_unique<QByteArray>();
    m_currentBuffer->reserve(m_bufferCapacity);
    m_writeBuffer->reserve(m_bufferCapacity);

    connect(m_timeoutTimer, &QTimer::timeout, this, &DownloadTask::onTimeout);
}

void DownloadTask::startNewTask(QThread* thread){
    this->moveToThread(thread);
    connect(thread, &QThread::started, this, &DownloadTask::startDownload);
}

void DownloadTask::resumeFromDB(QThread* thread, qint64 resPos){
    m_outputFile = new QFile(m_filePath, this);
    m_resumeDownloadPos = resPos;
    m_totalBytesWritten = m_resumeDownloadPos;
    this->moveToThread(thread);
    connect(thread, &QThread::started, this, &DownloadTask::resumeDownload);
}

void DownloadTask::startDownload(){
    if (m_outputFile && m_outputFile->isOpen()) {
        m_outputFile->close();
        m_outputFile->deleteLater();
        m_outputFile = nullptr;
    }

    m_outputFile = new QFile(m_filePath, this);

    if(!m_outputFile->open(QIODevice::WriteOnly)){
        qDebug() << "Помилка Не вдалося створити файл: " + m_filePath << "\n";
        return;
    }

    m_reply = m_manager->get(QNetworkRequest(m_url));

    m_timeoutTimer->start(m_timeoutSeconds * 1000);

    connect(m_reply, &QNetworkReply::readyRead, this, &DownloadTask::onReadyRead);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &DownloadTask::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &DownloadTask::onFinished);
    connect(m_reply, &QNetworkReply::errorOccurred, this, &DownloadTask::onError);
}

void DownloadTask::onReadyRead(){
    m_timeoutTimer->start(m_timeoutSeconds * 1000);

    qint64 bytesAvailable = m_reply->bytesAvailable();
    if(bytesAvailable > 0){
        QByteArray chunk = m_reply->read(bytesAvailable);

        m_currentBuffer->append(chunk);

        if(m_currentBuffer->size() >= m_bufferCapacity && !m_isWriting){
            swapBuffers();
            m_isWriting = true;
            flushBufferAsync();
        }
    }
}

void DownloadTask::flushBufferAsync(){
    if(m_writeBuffer->isEmpty() || !m_outputFile || !m_outputFile->isOpen()){
        m_isWriting = false;
        return;
    }

    qint64 bytesWritten = m_outputFile->write(*m_writeBuffer);

    if(bytesWritten == -1){
        qDebug() << "Write error:" << m_outputFile->errorString();
        emit error(m_outputFile->errorString());
        m_isWriting = false;
        return;
    }

    if(bytesWritten == m_writeBuffer->size()){
        m_writeBuffer->clear();
    }else if(bytesWritten > 0){
        *m_writeBuffer = m_writeBuffer->mid(bytesWritten);
    }

    m_totalBytesWritten += bytesWritten;
    m_outputFile->flush();

    if(!m_writeBuffer->isEmpty()){
        flushBufferAsync();
    }else{
        m_isWriting = false;
    }
}

void DownloadTask::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal){
    m_timeoutTimer->start(m_timeoutSeconds * 1000);

    measureSpeed(bytesReceived + m_resumeDownloadPos);

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

        adjustBufferSize();
        adjustTimeout();
    }
}

void DownloadTask::adjustBufferSize(){
    if (m_currentSpeed <= 0) return;

    int newBufferSize = m_bufferCapacity;

    if (m_currentSpeed < 1 * 1024 * 1024) {
        newBufferSize = 32 * 1024;
    }
    else if (m_currentSpeed < 3 * 1024 * 1024) {
        newBufferSize = 64 * 1024;
    }
    else if (m_currentSpeed < 6 * 1024 * 1024) {
        newBufferSize = 128 * 1024;
    }
    else {
        newBufferSize = 192 * 1024;
    }

    if (abs(newBufferSize - m_bufferCapacity) > 16 * 1024) {
        m_bufferCapacity = newBufferSize;

        qDebug() << "🔄 Buffer adapted:" << (m_bufferCapacity / 1024) << "KB"
                 << "Speed:" << (m_currentSpeed / 1024 / 1024) << "MB/s";

        if (m_currentBuffer) m_currentBuffer->reserve(m_bufferCapacity);
        if (m_writeBuffer) m_writeBuffer->reserve(m_bufferCapacity);
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

        qDebug() << "⏱️ Timeout adapted:" << m_timeoutSeconds << "seconds"
                 << "Speed:" << (m_currentSpeed / 1024 / 1024) << "MB/s";

        if (m_timeoutTimer->isActive()) {
            m_timeoutTimer->start(m_timeoutSeconds * 1000);
        }
    }

}

void DownloadTask::onFinished(){
    if(m_reply == nullptr){
        return;
    }

    if(m_reply->bytesAvailable() > 0){
        m_writeBuffer->append(m_reply->readAll());
    }

    if(!m_writeBuffer->isEmpty()){
        flushBufferAsync();
    }

    if(m_outputFile){
        if (m_outputFile->isOpen()) {
            m_outputFile->flush();
            m_outputFile->close();
        }
    }

    m_timeoutTimer->stop();

    bool success = (m_reply->error() == QNetworkReply::NoError);

    if (success) {
        qDebug() << "Файл успішно завантажено!";
        emit finished(m_url);
    } else{
        qDebug() << "Помилка завантаження:" << m_reply->errorString();
        emit error(m_reply->errorString());
    }
}

void DownloadTask::stopDownload(){
    if (m_reply) {
        m_reply->abort();
        m_reply = nullptr;
    }

    if (m_outputFile && m_outputFile->isOpen()) {
        m_outputFile->close();
    }

    emit finished(m_url);
}

void DownloadTask::pauseDownload(){
    m_isPaused = true;
    flushBufferAsync();
    if(m_reply){
        m_reply->abort();
    }
    m_resumeDownloadPos = m_totalBytesWritten;

    if (m_outputFile && m_outputFile->isOpen()) {
        m_outputFile->close();
    }
}

void DownloadTask::resumeDownload(){
    m_isPaused = false;
    qDebug() << "In resumeDowload\n";

    if (!QFile::exists(m_filePath)) {
        emit error("File not found for resume");
        return;
    }

    if(m_outputFile && !m_outputFile->isOpen()){
        m_outputFile->open(QIODevice::WriteOnly | QIODevice::Append);
    }

    QNetworkRequest request(m_url);
    QString range = QString("bytes=%1-").arg(m_resumeDownloadPos);
    request.setRawHeader("Range", range.toUtf8());

    m_reply = m_manager->get(request);

    connect(m_reply, &QNetworkReply::readyRead, this, &DownloadTask::onReadyRead);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &DownloadTask::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &DownloadTask::onFinished);
    connect(m_reply, &QNetworkReply::errorOccurred, this, &DownloadTask::onError);
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

void DownloadTask::onError(QNetworkReply::NetworkError errorCode){
    if (m_isHandlingError) return;
    m_isHandlingError = true;

    QString errorStr = m_reply ? m_reply->errorString() : "Unknown error";
    bool shouldRetry = isRetryableError(errorCode);

    handleFailure("Network error: " + errorStr, shouldRetry);
    m_isHandlingError = false;
}

void DownloadTask::onTimeout(){
    if(m_reply && m_reply->isRunning()){
        qWarning() << "Download timeout after" << m_timeoutSeconds << "seconds";
        handleFailure("Timeout after " + QString::number(m_timeoutSeconds) + " seconds", isRetryableError(QNetworkReply::TimeoutError));
    }
}

void DownloadTask::scheduleRetry(const QString& error){
    m_isAborting = true;
    ++m_retryCount;
    if(m_reply){
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    m_isAborting = false;

    m_resumeDownloadPos = m_totalBytesWritten;
    QTimer::singleShot(m_timeToRetry * 1000, this, &DownloadTask::resumeDownload);
    m_timeToRetry <<= 1;
}

void DownloadTask::handleFailure(const QString& errorContext, bool shouldRetry){
    if(m_isPaused) return;
    flushBufferAsync();
    if(shouldRetry && m_retryCount < MAX_RETRIES){
        scheduleRetry(errorContext);
    }else{
        emit error(errorContext + " after " + QString::number(m_retryCount) + " attempts");
        m_retryCount = 0;
    }
}

void DownloadTask::swapBuffers(){
    std::swap(m_currentBuffer, m_writeBuffer);
}

DownloadTask::~DownloadTask(){
    if(m_reply){
        m_reply->deleteLater();
    }

    if(m_outputFile){
        if(m_outputFile->isOpen()){
            m_outputFile->close();
        }
        m_outputFile->deleteLater();
    }
}
