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
#include "downloadrecord.h"

class DownloadItemAdapter;

class DownloadItem : public QWidget
{
    Q_OBJECT
public:
    DownloadItem(const QString&, const QString&, const QString&, QWidget* parent = nullptr);
    ~DownloadItem();
    QString getName() const;
    QString getUrl() const;
    QString getFilePath() const;
    void pauseDownloadAll(bool);
    void setNotChecked();
    void setFileName(const QString&);
    qint64 getResumePos() const;
    void updateFromDb(const DownloadRecord &record);
public slots:
    void onProgressChanged(qint64 bytesReceived, qint64 bytesTotal);
    void deleteItem();
    void onFinished();
    void chackWhatStatus(DownloadTask::Status);
private slots:
    void calculateSpeed();
    void calculateTimeToComplete();
    void onPauseCheckBox();
    void updateProgressChange();
    void onOpenFileInFolder();
signals:
    void statusChanged(DownloadTask::Status);
    void updateProgress();
    void deleteDownload(DownloadItem*);
    void ChangedBt(DownloadItem*, bool);
    void finishedDownload();
    void deleteFromDb(DownloadItem*);
private:
    QList<qint64> m_speedHistory;
    QString m_nameFileStr;
    QString m_filePath;
    QString m_url;
    QString m_sizeFileStr;
    QString m_timeToCompleteStr;
    qint64 m_lastBytesReceived;
    qint64 m_totalBytesReceived;
    qint64 m_currentSpeed;
    qint64 m_bytesTotal;
    qint64 m_timeToComplete;

    qint64 m_resumePosPercentages = 0;

    int m_percentages = 0;
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
    QLabel *m_timeToCompleteLabel;
    QProgressBar *m_progressBar;

    QProcess *m_process;
    QTimer *m_timer;

    void setUpUI();
    void setUpConnections();
    void setUpSpeedTimer();

    void updateSpeedString();
    void updateTimeToComplete();

    auto createSpacer(int) -> QWidget*;
    auto createExpandingSpacer() ->QWidget*;

    friend class DownloadAdapter;
};

#endif // DOWNLOADITEM_H
