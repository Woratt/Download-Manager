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

class DownloadItemAdapter;

class DownloadItem : public QWidget
{
    Q_OBJECT
public:
    DownloadItem(const QString&, const QString&, QWidget* parent = nullptr);
    ~DownloadItem();
    QString getUrl();
    QString getFilePath();
    QString getDir();
    bool isChecked();
    void pauseDownloadAll(bool);
    void setNotChecked();
    QString& getStatus();
    bool isFromDB();
    qint64 getResumePos();
    void setStatus(const QString&);
public slots:
    void onProgressChanged(qint64 bytesReceived, qint64 bytesTotal);
    void deleteItem();
    void onFinished();
private slots:
    void calculateSpeed();
    void onPauseCheckBox();
    void updateProgressChange();
    void onOpenFileInFolder();
signals:
    void updateProgress();
    void pauseDownload();
    void resumeDownload();
    void deleteDownload(DownloadItem*);
    void ChangedBt(DownloadItem*, bool);
private:
    QString m_nameFileStr;
    QString m_url;
    QString m_dir;
    QString m_sizeFileStr;
    QString m_status = "downloading";
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
