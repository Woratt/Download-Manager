#ifndef DOWNLOADTYPES_H
#define DOWNLOADTYPES_H

#include <QString>
#include <QVector>

class DownloadItem;

namespace DownloadTypes {
// Enum для типів конфліктів
enum ConflictType {
    NoConflict,      // Немає конфліктів
    FileExists,      // Файл вже існує на диску
    UrlDownloading,  // Цей URL вже завантажується
    BothConflicts    // Обидва конфлікти
};

enum Action {
    Cancel,
    Download,
    DownloadWithNewName,
    Undetermined
};

struct UserChoice {
    Action action = Undetermined;
    QString newFileName; // Для перейменування
};


struct ConflictResult {
    ConflictType type = NoConflict;
    QString filePath;
    bool existingDownloads;
};



}

#endif // DOWNLOADTYPES_H
