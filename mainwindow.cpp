#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto central = new QWidget(this);

    setCentralWidget(central);
    setMinimumSize(640, 400);

    m_downloadManager = new DownloadManager(this);

    setUpUI();
    setUpConnections();

    m_downloadManager->setItemsFromDB();
}

MainWindow::~MainWindow() {}

void MainWindow::closeEvent(QCloseEvent* event){
    emit close();

    event->accept();
}

void MainWindow::setUpUI(){
    QWidget *central = centralWidget();

    m_vboxLayout = new QVBoxLayout(central);
    m_layoutForSelectedItemsBt = new QHBoxLayout();

    m_nameApp = new QLabel("Download Manager");

    m_urlInput = new QLineEdit(this);
    m_urlInput->setPlaceholderText("Enter the link");

    m_downloadButton = new QPushButton("Download");
    m_browseButton = new QPushButton("Select a folder");

    m_downloadAllItemsBt = new QPushButton("Download All");
    m_pauseAllItemsBt = new QPushButton("Pause All");
    m_deleteAllItemsBt = new QPushButton("Delete All");

    m_layoutForSelectedItemsBt->addWidget(m_downloadAllItemsBt);
    m_layoutForSelectedItemsBt->addWidget(m_pauseAllItemsBt);
    m_layoutForSelectedItemsBt->addWidget(m_deleteAllItemsBt);

    m_hboxLayout = new QHBoxLayout();

    m_listWidget = new QListWidget();
    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);

    m_hboxLayout->addWidget(m_urlInput);
    m_hboxLayout->addWidget(m_browseButton);

    m_vboxLayout->addWidget(m_nameApp);
    m_vboxLayout->addLayout(m_hboxLayout);
    m_vboxLayout->addWidget(m_downloadButton);
    m_vboxLayout->addLayout(m_layoutForSelectedItemsBt);
    m_vboxLayout->addWidget(m_listWidget);

    setLayoutForSelectedItemsBtVisible(false);
}

void MainWindow::setUpConnections(){
    connect(m_downloadButton, &QPushButton::clicked, this, &MainWindow::onClickDownloadButton);

    connect(m_browseButton, &QPushButton::clicked, this, &MainWindow::onClickBrowseButton);

    connect(m_downloadManager, &DownloadManager::downloadReadyToAdd, this, &MainWindow::addDownloadItem);
    connect(this, &MainWindow::close, m_downloadManager, &DownloadManager::saveAll);

    connect(m_downloadManager, &DownloadManager::showButtons, this, [=](){
        setLayoutForSelectedItemsBtVisible(true);
    });
    connect(m_downloadManager, &DownloadManager::hideButtons, this, [=](){
        setLayoutForSelectedItemsBtVisible(false);
    });
    connect(m_downloadManager, &DownloadManager::setDownloadItemFromDB, this, &MainWindow::addDownloadItem);
    connect(m_downloadManager, &DownloadManager::conflictsDetected, this, &MainWindow::handleDownloadConflicts);

    connect(m_downloadAllItemsBt, &QPushButton::clicked, m_downloadManager, &DownloadManager::downloadAll);
    connect(m_pauseAllItemsBt, &QPushButton::clicked, m_downloadManager, &DownloadManager::pauseAll);
    connect(m_deleteAllItemsBt, &QPushButton::clicked, m_downloadManager, &DownloadManager::deleteAll);

}

void MainWindow::handleDownloadConflicts(const QString &url, const DownloadTypes::ConflictResult &conflict){
    qDebug() << conflict.type;
    DownloadTypes::UserChoice choice = showConflictDialog(url, conflict.type);

    m_downloadManager->processDownloadRequest(url, m_dir, choice);

}

DownloadTypes::UserChoice MainWindow::showConflictDialog(const QString &url, DownloadTypes::ConflictType type) {

    DownloadTypes::UserChoice choice;
    choice.action = DownloadTypes::Cancel;

    // Просте питання
    QString question;
    switch (type) {
    case DownloadTypes::FileExists:
        question = "File already exists. Download and replace?";
        break;
    case DownloadTypes::UrlDownloading:
        question = "URL already downloading. Download another copy?";
        break;
    case DownloadTypes::BothConflicts:
        question = "File exists and URL downloading. Download anyway?";
        break;
    case DownloadTypes::NoConflict:
        question = "Download this file?";
        break;
    }

    QString message = QString("%1\n\nURL: %2").arg(question).arg(url);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Download Manager",
        message,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        choice.action = DownloadTypes::Download;

        // Якщо конфлікт файлу, питаємо чи перейменувати
        if (type == DownloadTypes::FileExists || type == DownloadTypes::BothConflicts) {
            QMessageBox renameBox;
            renameBox.setWindowTitle("File Name");
            renameBox.setText("Do you want to rename the file?");
            renameBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            renameBox.setDefaultButton(QMessageBox::No);

            if (renameBox.exec() == QMessageBox::Yes) {
                choice.action = DownloadTypes::Undetermined;
                QString newName = QInputDialog::getText(
                    this, "New Name", "Enter file name:", QLineEdit::Normal, ""
                    );
                if (!newName.isEmpty()) {
                    choice.newFileName = newName;
                    choice.action = DownloadTypes::DownloadWithNewName;
                } else {
                    choice.action = DownloadTypes::Download; // Повертаємо до Download
                }
            }
        }
    }

    return choice;
}

void MainWindow::onClickDownloadButton(){
    QString url = m_urlInput->text().trimmed();

    if(url.isEmpty() || m_dir.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter URL and select folder");
        return;
    }

    if(!(url.startsWith("http://") || url.startsWith("https://"))){
        QMessageBox::warning(this, "Error", "Please enter a valid HTTP/HTTPS URL");
        return;
    }

    m_urlInput->clear();

    DownloadTypes::UserChoice choice;

    m_downloadManager->processDownloadRequest(url, m_dir, choice);
}

QString MainWindow::askForFileAction(const QString &originalPath) {
    /*QMessageBox msgBox;
    msgBox.setWindowTitle("File name conflict");
    msgBox.setText(QString("File '%1' exists.").arg(info.fileName()));
    msgBox.setInformativeText("Select an action:");

    QPushButton *replaceBtn = msgBox.addButton("Replace", QMessageBox::DestructiveRole);
    QPushButton *renameBtn = msgBox.addButton("Rename", QMessageBox::ActionRole);
    QPushButton *cancelBtn = msgBox.addButton("Cencelled", QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == replaceBtn) {
        QString fileName = info.fileName();
        return fileName;
    } else if (msgBox.clickedButton() == renameBtn) {
        bool ok;
        QString newName = QInputDialog::getText(nullptr,
                                                "Rename file",
                                                "Enter a new file name:",
                                                QLineEdit::Normal,
                                                info.fileName(),
                                                &ok);

        if (ok && !newName.isEmpty()) {
            return newName + "." + info.suffix();
        }
    }

    return QString();*/
}

void MainWindow::onClickBrowseButton(){
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    "select a folder",
                                                    m_dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty()){
        m_dir = dir;
    }
}

void MainWindow::addDownloadItem(DownloadItem* item){

    QListWidgetItem *listItem = new QListWidgetItem("");

    m_listWidget->addItem(listItem);
    m_listWidget->setItemWidget(listItem, item);

    listItem->setSizeHint(QSize(item->width(), item->height()));

    connect(item, &DownloadItem::deleteDownload, this, &MainWindow::deleteDownloadItem);

}

void MainWindow::deleteDownloadItem(DownloadItem* item){
    for(int i = 0; i < m_listWidget->count(); ++i){
        QListWidgetItem* listItem = m_listWidget->item(i);
        DownloadItem* downloadItem = qobject_cast<DownloadItem*>(m_listWidget->itemWidget(listItem));
        if(downloadItem == item){
            delete m_listWidget->takeItem(i);
            break;
        }
    }
}

void MainWindow::setLayoutForSelectedItemsBtVisible(bool visible)
{
    m_downloadAllItemsBt->setVisible(visible);
    m_pauseAllItemsBt->setVisible(visible);
    m_deleteAllItemsBt->setVisible(visible);
}
