#ifndef DOWNLOADTYPES_H
#define DOWNLOADTYPES_H

#include <QString>
#include <QVector>

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

struct FileInfo{
    QString filePath;
    QString fileName;
    qint64 fileSize;
    qint64 quantityOfChunks{8};

    bool operator<(const FileInfo &other) const {
        return filePath < other.filePath;
    }

    bool operator==(const FileInfo &other) const {
        if(other.fileName == fileName && other.filePath == filePath && other.fileSize == fileSize) return true;
        else return false;
    }
};



}

#endif // DOWNLOADTYPES_H
