#ifndef DOWNLOADTYPES_H
#define DOWNLOADTYPES_H

#include <QString>
#include <QVector>
#include <QUuid>

class DownloadItem;

namespace DownloadTypes {

enum ConflictType {
    NoConflict,
    FileExists,
    UrlDownloading,
    BothConflicts
};

enum Action {
    Cancel,
    Download,
    DownloadWithNewName,
    Undetermined
};

struct UserChoice {
    Action action = Undetermined;
    QString newFileName;
};


struct ConflictResult {
    ConflictType type = NoConflict;
    QString filePath;
    bool existingDownloads;
};

enum class DownloadStatus { Preparing, Ready, Pending, Downloading, Paused, Error, Completed, Cancelled };

struct DownloadRecord {
    QUuid id;
    QString name;
    QString url;
    QString filePath;
    QString expectedHash;
    QString actualHash;
    QString hashAlgorithm;
    QVector<QByteArray> chunkHashes;
    DownloadStatus status = DownloadStatus::Preparing;
    qint64 totalBytes = 0;
    qint64 downloadedBytes = 0;
    qint64 quantityOfChunks = 8;

    bool operator==(const DownloadRecord& other) const {
        return id == other.id &&
               name == other.name &&
               url == other.url &&
               filePath == other.filePath &&
               expectedHash == other.expectedHash &&
               actualHash == other.actualHash &&
               hashAlgorithm == other.hashAlgorithm &&
               chunkHashes == other.chunkHashes &&
               status == other.status &&
               totalBytes == other.totalBytes &&
               downloadedBytes == other.downloadedBytes &&
               quantityOfChunks == other.quantityOfChunks;
    }

    bool operator!=(const DownloadRecord& other) const {
        return !(*this == other);
    }

    bool operator<(const DownloadRecord& other) const {
        if (id != other.id) {
            return id < other.id;
        }

        if (name != other.name) return name < other.name;
        if (url != other.url) return url < other.url;
        if (filePath != other.filePath) return filePath < other.filePath;
        if (status != other.status) return static_cast<int>(status) < static_cast<int>(other.status);
        if (totalBytes != other.totalBytes) return totalBytes < other.totalBytes;
        if (downloadedBytes != other.downloadedBytes) return downloadedBytes < other.downloadedBytes;
        if (quantityOfChunks != other.quantityOfChunks) return quantityOfChunks < other.quantityOfChunks;

        return false;
    }

    bool operator>(const DownloadRecord& other) const  { return other < *this; }
    bool operator<=(const DownloadRecord& other) const { return !(*this > other); }
    bool operator>=(const DownloadRecord& other) const { return !(*this < other); }
};



}

#endif // DOWNLOADTYPES_H
