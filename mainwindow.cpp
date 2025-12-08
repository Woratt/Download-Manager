#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto central = new QWidget(this);

    setCentralWidget(central);
    setMinimumSize(640, 400);

    m_downloadManager = new DownloadManager(this);

    m_dir = QDir::homePath();
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

    connect(this, &MainWindow::startDownload, m_downloadManager, &DownloadManager::startDownload);
    connect(this, &MainWindow::close, m_downloadManager, &DownloadManager::saveAll);

    connect(m_downloadManager, &DownloadManager::showButtons, this, [=](){
        setLayoutForSelectedItemsBtVisible(true);
    });
    connect(m_downloadManager, &DownloadManager::hideButtons, this, [=](){
        setLayoutForSelectedItemsBtVisible(false);
    });
    connect(m_downloadManager, &DownloadManager::setDownloadItemFromDB, this, &MainWindow::addDownloadItem);

    connect(m_downloadAllItemsBt, &QPushButton::clicked, m_downloadManager, &DownloadManager::downloadAll);
    connect(m_pauseAllItemsBt, &QPushButton::clicked, m_downloadManager, &DownloadManager::pauseAll);
    connect(m_deleteAllItemsBt, &QPushButton::clicked, m_downloadManager, &DownloadManager::deleteAll);
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

    DownloadItem *item = new DownloadItem(url, m_dir, this);
    addDownloadItem(item);

    emit startDownload(item);
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
