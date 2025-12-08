#include "downloaditem.h"

DownloadItem::DownloadItem(const QString& url, const QString& dir, QWidget *parent) : QWidget(parent),
                                                                                    m_url(url),
                                                                                    m_dir(dir),
                                                                                    m_nameFileStr(getFileNameFromUrl(url)),
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

    m_openInFolderButton = new QPushButton("Open in Folder");
    m_deleteButton = new QPushButton("Delete");

    m_progresSize = new QLabel("0%");
    m_speedDownload = new QLabel("0B/s");
    m_statusDownload = new QLabel("Download");
    m_sizeFile = new QLabel();

    m_mainLayout = new QHBoxLayout(this);
    m_vLayout = new QVBoxLayout();
    m_upperLayout = new QHBoxLayout();
    m_lowerLayout = new QHBoxLayout();

    m_upperLayout->addWidget(m_nameFile);
    m_upperLayout->addWidget(m_openInFolderButton);
    m_upperLayout->addWidget(m_deleteButton);

    m_lowerLayout->addWidget(m_pauseCheckBox);
    m_lowerLayout->addWidget(createSpacer(20));
    m_lowerLayout->addWidget(m_statusDownload);
    m_lowerLayout->addWidget(m_speedDownload);
    m_lowerLayout->addWidget(m_sizeFile);
    m_lowerLayout->addWidget(m_progresSize);

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
    connect(m_deleteButton, &QPushButton::clicked, this, &DownloadItem::deleteItem);

    connect(m_checkBox, &QCheckBox::checkStateChanged, this, [=](bool checked){
        m_isChecked = checked;
        emit ChangedBt(this, m_isChecked);
    });
}

void DownloadItem::onPauseCheckBox()
{
    if(m_pauseCheckBox->isChecked()){
        m_statusDownload->setText("Download");
        setStatus("Downloading");
        emit resumeDownload();
    }else{
        m_statusDownload->setText("Pause");
        setStatus("Paused");
        emit pauseDownload();
    }
}

void DownloadItem::pauseDownloadAll(bool checked){
    if(checked){
        m_pauseCheckBox->setChecked(checked);
        m_statusDownload->setText("Download");
    }else{
        m_pauseCheckBox->setChecked(checked);
        m_statusDownload->setText("Pause");
    }
}

QString DownloadItem::getFileNameFromUrl(const QUrl& url){
    QString path = url.path();
    QString fileName = QFileInfo(path).fileName();

    if(fileName.isEmpty()){
        fileName = "downloaded_file.txt";
    }
    return fileName;
}

void DownloadItem::setUpSpeedTimer(){
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DownloadItem::calculateSpeed);
    m_timer->start(1000);
}

void DownloadItem::calculateSpeed(){
    if(m_totalBytesReceived > 0){
        qint64 bytesDiff = m_totalBytesReceived - m_lastBytesReceived;
        m_currentSpeed = bytesDiff;

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

QString DownloadItem::getUrl(){
    return m_url;
}

QString DownloadItem::getFilePath(){
    return m_dir + "/" + m_nameFileStr;
}

QString DownloadItem::getDir(){
    return m_dir;
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
        qDebug() << "Файл не існує!";
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
        qDebug() << "Процес не завершився вчасно";
    }
}

void DownloadItem::setNotChecked(){
    m_checkBox->setChecked(false);
}

void DownloadItem::deleteItem(){
    emit deleteDownload(this);
}

bool DownloadItem::isChecked(){
    return m_isChecked;
}

void DownloadItem::onFinished(){
    m_timer->deleteLater();
    setStatus("completed");
}

DownloadItem::~DownloadItem(){
    m_process->deleteLater();
}

QString& DownloadItem::getStatus(){
    return m_status;
}

bool DownloadItem::isFromDB(){
    return m_fromDB;
}

qint64 DownloadItem::getResumePos(){
    return m_totalBytesReceived;
}

void DownloadItem::setStatus(const QString& status){
    m_status = status;
}


