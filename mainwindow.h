#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QLayout>
#include <QFileDialog>
#include <QThread>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QTableWidget>
#include <QList>
#include <QListWidget>

#include "downloadmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void clickDownoaldButton();
    void startDownload(DownloadItem*);
    void close();
private:
    // UI and connections functions
    void setUpUI();
    void setUpConnections();

    // Network
    DownloadManager *m_downloadManager;
    QThread *thread;

    // UI elements
    QLabel *m_nameApp;
    QLineEdit *m_urlInput;
    QPushButton *m_downloadButton;
    QPushButton *m_browseButton;
    QPushButton *m_downloadAllItemsBt;
    QPushButton *m_pauseAllItemsBt;
    QPushButton *m_deleteAllItemsBt;
    QHBoxLayout *m_layoutForSelectedItemsBt;
    QVBoxLayout *m_vboxLayout;
    QHBoxLayout *m_hboxLayout;

    // File path directory
    QString m_dir;

    QListWidget *m_listWidget;

    void setLayoutForSelectedItemsBtVisible(bool);

protected:
    void closeEvent(QCloseEvent*) override;
private slots:
    void onClickDownloadButton();
    void onClickBrowseButton();
    void deleteDownloadItem(DownloadItem*);
public slots:
    void addDownloadItem(DownloadItem*);
};
#endif // MAINWINDOW_H
