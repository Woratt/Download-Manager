#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

#include <QObject>
#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QFileInfo>
#include <QApplication>
#include <QListWidget>
#include <QTimer>
#include <QToolButton>
#include <QProcess>

#include "toogle.h"
#include "downloadtask.h"

class DownloadItemAdapter;

class DownloadItem : public QWidget
{
    Q_OBJECT
public:
    DownloadItem(const QString&, const QString&, QWidget* parent = nullptr, const QString& name = "");
    ~DownloadItem();
    QString getUrl();
    QString getFilePath();
    bool isChecked();
    void pauseDownloadAll(bool);
    void setNotChecked();
    bool isFromDB();
    void setFileName(const QString&);
    qint64 getResumePos();
public slots:
    void onProgressChanged(qint64 bytesReceived, qint64 bytesTotal);
    void deleteItem();
    void onFinished();
    void chackWhatStatus(DownloadTask::Status);
private slots:
    void calculateSpeed();
    void onPauseCheckBox();
    void updateProgressChange();
    void onOpenFileInFolder();
signals:
    void statusChanged(DownloadTask::Status);
    void updateProgress();
    void pauseDownload();
    void resumeDownload();
    void deleteDownload(DownloadItem*);
    void ChangedBt(DownloadItem*, bool);
    void finishedDownload();
private:
    QString m_nameFileStr;
    QString m_filePath;
    QString m_url;
    QString m_sizeFileStr;
    qint64 m_lastBytesReceived;
    qint64 m_totalBytesReceived;
    qint64 m_currentSpeed;
    qint64 m_bytesTotal;

    qint64 m_resumePosPercentages = 0;

    int m_percentages = 0;
    bool m_isChecked = false;
    bool m_fromDB = false;

    QVBoxLayout *m_vLayout;
    QHBoxLayout *m_upperLayout;
    QHBoxLayout *m_lowerLayout;
    QHBoxLayout *m_mainLayout;

    QCheckBox *m_checkBox;
    Toogle *m_pauseCheckBox;
    QPushButton *m_deleteButton;
    QPushButton *m_openInFolderButton;
    QPushButton *m_cancellButton;
    QLabel *m_nameFile;
    QLabel *m_progresSize;
    QLabel *m_speedDownload;
    QLabel *m_statusDownload;
    QLabel *m_sizeFile;
    QProgressBar *m_progressBar;

    QProcess *m_process;
    QTimer *m_timer;

    void setUpUI();
    void setUpConnections();
    void setUpSpeedTimer();

    void updateSpeedString();

    QString getFileNameFromUrl(const QUrl&);
    auto createSpacer(int) -> QWidget*;
    auto createExpandingSpacer() ->QWidget*;

    friend class DownloadItemAdapter;
};

#endif // DOWNLOADITEM_H
