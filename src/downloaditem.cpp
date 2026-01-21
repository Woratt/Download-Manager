#include "../headers/downloaditem.h"

DownloadItem::DownloadItem(const QString& url, const QString& filePath, const QString& name, QWidget *parent) : QWidget(parent),
                                                                                    m_url(url),
                                                                                    m_filePath(filePath),
                                                                                    m_nameFileStr(name),
                                                                                    m_lastBytesReceived(0),
                                                                                    m_currentSpeed(0){
    m_process = new QProcess(this);
    setUpUI();
    setUpConnections();
    setUpSpeedTimer();
}


void DownloadItem::setUpUI(){
    m_nameFile = new QLabel(m_nameFileStr);
    m_progressBar = new QProgressBar();
    m_checkBox = new QCheckBox();

    m_pauseCheckBox = new Toogle(25, Qt::gray, Qt::white, Qt::blue);
    m_pauseCheckBox->setChecked(true);
    m_pauseCheckBox->hide();

    m_openInFolderButton = new QPushButton("Open in Folder");
    m_deleteButton = new QPushButton("Delete");
    m_cancellButton = new QPushButton("Cancell");

    m_openInFolderButton->setMaximumWidth(200);
    m_deleteButton->setMaximumWidth(200);
    m_cancellButton->setMaximumWidth(200);

    m_progresSize = new QLabel("0%");
    m_speedDownload = new QLabel("0B/s");
    m_statusDownload = new QLabel("Preparing");
    m_sizeFile = new QLabel();
    m_timeToCompleteLabel = new QLabel();

    m_mainLayout = new QHBoxLayout(this);
    m_vLayout = new QVBoxLayout();
    m_upperLayout = new QHBoxLayout();
    m_lowerLayout = new QHBoxLayout();

    m_upperLayout->addWidget(m_nameFile);
    m_upperLayout->addWidget(m_openInFolderButton);
    m_upperLayout->addWidget(m_cancellButton);
    m_upperLayout->addWidget(m_deleteButton);

    m_lowerLayout->addWidget(m_pauseCheckBox);
    m_lowerLayout->addWidget(createSpacer(20));
    m_lowerLayout->addWidget(m_statusDownload);
    m_lowerLayout->addWidget(createExpandingSpacer());
    m_lowerLayout->addWidget(m_speedDownload);
    m_lowerLayout->addWidget(createExpandingSpacer());
    m_lowerLayout->addWidget(m_sizeFile);
    m_lowerLayout->addWidget(createExpandingSpacer());
    m_lowerLayout->addWidget(m_progresSize);
    m_lowerLayout->addWidget(createExpandingSpacer());
    m_lowerLayout->addWidget(m_timeToCompleteLabel);
    m_lowerLayout->addWidget(createSpacer(20));

    m_vLayout->setSpacing(0);

    m_vLayout->addLayout(m_upperLayout);
    m_vLayout->addWidget(m_progressBar);
    m_vLayout->addLayout(m_lowerLayout);

    m_mainLayout->addWidget(m_checkBox);
    m_mainLayout->addLayout(m_vLayout);
}

void DownloadItem::setUpConnections()
{
    connect(m_pauseCheckBox, &Toogle::checkStateChanged, this, &DownloadItem::onPauseCheckBox);
    connect(this, &DownloadItem::updateProgress, this, &DownloadItem::updateProgressChange);

    connect(m_openInFolderButton, &QPushButton::clicked, this, &DownloadItem::onOpenFileInFolder);
    connect(m_cancellButton, &QPushButton::clicked, this, [=](){
        emit statusChanged(DownloadTask::Status::Cancelled);
    });
    connect(m_deleteButton, &QPushButton::clicked, this, [=](){
        emit statusChanged(DownloadTask::Status::Deleted);
        emit deleteDownload(this);
    });

    connect(m_checkBox, &QCheckBox::checkStateChanged, this, [=](bool checked){
        emit ChangedBt(this, checked);
    });
}

void DownloadItem::updateFromDb(const DownloadRecord &record)
{
    m_nameFileStr = record.m_name;
    m_filePath = record.m_filePath;
    m_url = record.m_url;
    m_bytesTotal = record.m_totalBytes;
    m_pauseCheckBox->show();
    QString status = record.m_status;
    if (record.m_status == "pending") chackWhatStatus(DownloadTask::Pending);
    if (record.m_status == "downloading") chackWhatStatus(DownloadTask::Downloading);
    if (record.m_status == "resumed") chackWhatStatus(DownloadTask::Resumed);
    if (record.m_status == "start_new_task") chackWhatStatus(DownloadTask::StartNewTask);
    if (record.m_status == "resumed_in_pending") chackWhatStatus(DownloadTask::ResumedInPending);
    if (record.m_status == "resumed_in_downloading") chackWhatStatus(DownloadTask::ResumedInDownloading);
    if (record.m_status == "paused") chackWhatStatus(DownloadTask::Paused);
    if (record.m_status == "paused_new") chackWhatStatus(DownloadTask::PausedNew);
    if (record.m_status == "paused_resume") chackWhatStatus(DownloadTask::PausedResume);
    if (record.m_status == "error") chackWhatStatus(DownloadTask::Error);
    if (record.m_status == "cancelled") chackWhatStatus(DownloadTask::Cancelled);
    if (record.m_status == "deleted") chackWhatStatus(DownloadTask::Deleted);
    if(record.m_status == "preparing") chackWhatStatus(DownloadTask::Preparing);
    if(record.m_status == "prepared") chackWhatStatus(DownloadTask::Prepared);
    if (record.m_status == "completed"){
        onProgressChanged(record.m_totalBytes, record.m_totalBytes);
        chackWhatStatus(DownloadTask::Completed);
    }else{
        onProgressChanged(record.m_downloadedBytes, record.m_totalBytes);
    }
}

void DownloadItem::onPauseCheckBox()
{
    if(m_pauseCheckBox->isChecked()){
        emit statusChanged(DownloadTask::Status::Resumed);
    }else{
        emit statusChanged(DownloadTask::Status::Paused);
    }
}

void DownloadItem::pauseDownloadAll(bool checked){
    if(checked){
        m_pauseCheckBox->setChecked(checked);
        emit statusChanged(DownloadTask::Status::Resumed);
    }else{
        m_pauseCheckBox->setChecked(checked);
        emit statusChanged(DownloadTask::Status::Paused);
    }
}

void DownloadItem::chackWhatStatus(DownloadTask::Status status){
    switch (status) {
    case DownloadTask::Status::Pending:
        m_pauseCheckBox->setChecked(true);
        m_statusDownload->setText("Pending");
        break;
    case DownloadTask::Status::Cancelled:
        m_statusDownload->setText("Cancelled");
        m_pauseCheckBox->hide();
        m_cancellButton->hide();
        emit finishedDownload();
        break;
    case DownloadTask::Status::Error:
        m_statusDownload->setText("Error");
        m_pauseCheckBox->hide();
        emit finishedDownload();
        break;
    case DownloadTask::Status::Completed:
        m_statusDownload->setText("Completed");
        m_pauseCheckBox->hide();
        m_cancellButton->hide();
        emit finishedDownload();
        break;
    case DownloadTask::Status::ResumedInDownloading:
        m_statusDownload->setText("Downloading");
        break;
    case DownloadTask::Status::ResumedInPending:
        m_statusDownload->setText("Pending");
        break;
    case DownloadTask::Status::Paused:
        m_statusDownload->setText("Paused");
        m_pauseCheckBox->setChecked(false);
        break;
    case DownloadTask::Status::Downloading:
        m_pauseCheckBox->show();
        m_pauseCheckBox->setChecked(true);
        m_statusDownload->setText("Downloading");
        break;
    case DownloadTask::Status::Resumed:
        m_statusDownload->setText("Downloading");
        m_pauseCheckBox->setChecked(true);
        break;
    case DownloadTask::Status::StartNewTask:
        m_statusDownload->setText("Downloading");
        m_pauseCheckBox->setChecked(true);
        break;
    case DownloadTask::Status::PausedNew:
        m_statusDownload->setText("Paused");
        m_pauseCheckBox->setChecked(false);
        break;
    case DownloadTask::Status::PausedResume:
        break;
    case DownloadTask::Status::Preparing:
        m_statusDownload->setText("Preparing");
        m_pauseCheckBox->hide();
        break;
    case DownloadTask::Status::Prepared:
        m_statusDownload->setText("Prepared");
        m_pauseCheckBox->hide();
        break;
    case DownloadTask::Status::Deleted:
        m_statusDownload->setText("Deleted");
        m_pauseCheckBox->hide();
        m_cancellButton->hide();
        break;
    case DownloadTask::Status::FileIntegrityCheck:
        m_statusDownload->setText("File Integrity Checking");
        m_pauseCheckBox->hide();
        m_cancellButton->hide();
        break;
    }
}

void DownloadItem::setUpSpeedTimer(){
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DownloadItem::calculateSpeed);
    connect(m_timer, &QTimer::timeout, this, &DownloadItem::calculateTimeToComplete);
    m_timer->start(1000);
}

void DownloadItem::calculateSpeed(){
    if(m_totalBytesReceived > 0){
        qint64 bytesDiff = m_totalBytesReceived - m_lastBytesReceived;
        m_currentSpeed = bytesDiff;
        m_speedHistory.append(m_currentSpeed);

        if (m_speedHistory.size() > 5) {
            m_speedHistory.removeFirst();
        }
        qint64 sumSpeed = 0;
        for (qint64 s : m_speedHistory) sumSpeed += s;
        m_currentSpeed = sumSpeed / m_speedHistory.size();

        updateSpeedString();
    }
    m_lastBytesReceived = m_totalBytesReceived;
}

void DownloadItem::updateSpeedString(){
    QString speedText;

    if(m_currentSpeed < 1024){
        speedText = QString("%1 B/s").arg(m_currentSpeed);
    }else if(m_currentSpeed < 1024 * 1024){
        speedText = QString("%1 KB/s").arg(m_currentSpeed / 1024);
    }else {
        speedText = QString("%1 MB/s").arg(m_currentSpeed / (1024.0 * 1024.0), 0, 'f', 1);
    }

    m_speedDownload->setText(speedText);
}

void DownloadItem::setFileName(const QString& newFileName)
{
    m_nameFileStr = newFileName;
    m_nameFile->setText(m_nameFileStr);
}

QString DownloadItem::getName()
{
    return m_nameFileStr;
}

QString DownloadItem::getUrl(){
    return m_url;
}

QString DownloadItem::getFilePath(){
    return m_filePath;
}

void DownloadItem::onProgressChanged(qint64 bytesReceived, qint64 bytesTotal){
    m_bytesTotal = bytesTotal;

    if(m_bytesTotal > 0){
        m_percentages = static_cast<int>((bytesReceived * 100) / bytesTotal);
    }

    m_totalBytesReceived = bytesReceived;

    if(bytesTotal < 1024){
        m_sizeFileStr = QString("%1/%2 B").arg(bytesReceived).arg(bytesTotal);
    }else if(bytesTotal < 1024 * 1024){
        m_sizeFileStr = QString("%1/%2 KB")
                            .arg(bytesReceived / 1024.0, 0, 'f', 1)
                            .arg(bytesTotal / 1024.0, 0, 'f', 1);
    }else if(bytesTotal < 1024 * 1024 * 1024){
        m_sizeFileStr = QString("%1/%2 MB")
                            .arg(bytesReceived / (1024.0 * 1024.0), 0, 'f', 1)
                            .arg(bytesTotal / (1024.0 * 1024.0), 0, 'f', 1);
    }else{
        m_sizeFileStr = QString("%1/%2 GB")
                            .arg(bytesReceived / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2)
                            .arg(bytesTotal / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }
    emit updateProgress();
}

void DownloadItem::updateProgressChange(){
    m_progressBar->setValue(m_percentages);

    m_progresSize->setText(QString::number(m_percentages) + "%");

    m_sizeFile->setText(m_sizeFileStr);
}

auto DownloadItem::createExpandingSpacer() -> QWidget*
{
    auto *spaser = new QWidget();
    spaser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return spaser;
}

auto DownloadItem::createSpacer(int width) -> QWidget*
{
    auto *spaser = new QWidget();
    spaser->setFixedWidth(width);
    return spaser;
}

void DownloadItem::onOpenFileInFolder(){
    QFileInfo info(getFilePath());
    if (!info.exists()) {
        //!!!
        return;
    }

    #ifdef Q_OS_MAC
    m_process->start("open", QStringList() << "-R" << getFilePath());
    #elif defined(Q_OS_WIN)
        m_process->start("cmd", QStringList() << "/c" << "start" << getFilePath());
    #elif defined(Q_OS_LINUX)
        m_process->start("xdg-open", QStringList() << getFilePath());
    #endif

    if (!m_process->waitForFinished(5000)) {
        //!!!
    }
}

void DownloadItem::calculateTimeToComplete(){
    long long size = m_bytesTotal - m_totalBytesReceived;
    if(m_currentSpeed > 0){
        m_timeToComplete = size/m_currentSpeed;
    }else{
        m_timeToComplete = 0;
    }
    updateTimeToComplete();
}

void DownloadItem::updateTimeToComplete(){
    if(m_timeToComplete == 0) m_timeToCompleteStr = "0s";
    else{
        qint64 hours = m_timeToComplete / 3600;
        qint64 minutes = (m_timeToComplete % 3600) / 60;
        qint64 secs = m_timeToComplete % 60;

        if (hours > 0) {
            m_timeToCompleteStr = QString("%1h %2m")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'));
        }else if(minutes > 0){
            m_timeToCompleteStr = QString("%1m %2s")
            .arg(minutes, 2, 10, QChar('0'))
                .arg(secs, 2, 10, QChar('0'));
        }else{
            m_timeToCompleteStr = QString("%1s")
            .arg(secs, 2, 10, QChar('0'));
        }
    }
    m_timeToCompleteLabel->setText(m_timeToCompleteStr);
}

void DownloadItem::setNotChecked(){
    m_checkBox->setChecked(false);
}

void DownloadItem::deleteItem(){
    emit deleteDownload(this);
}

void DownloadItem::onFinished(){
    m_timer->deleteLater();
}

DownloadItem::~DownloadItem(){
    m_process->deleteLater();
}

qint64 DownloadItem::getResumePos(){
    return m_totalBytesReceived;
}
