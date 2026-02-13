#include <gtest/gtest.h>
#include <QtTest/QSignalSpy>
#include "downloaddatabase.h"

class DownloadDatabaseTest : public ::testing::Test {
protected:
    DownloadDatabase* db;

    void SetUp() override
    {
        db = new DownloadDatabase(":memory:");
    }

    void TearDown() override
    {
        delete db;
    }

    QString callGetDatabasePath() {
        return db->getSystemDatabasePath();
    }

    bool callIsValidPath(const QString& path)
    {
        return db->isValidPath(path);
    }

};

TEST_F(DownloadDatabaseTest, DatabaseSchemaIsCorrect)
{
    QSqlDatabase sqliteDb = QSqlDatabase::database();

    ASSERT_TRUE(sqliteDb.tables().contains("downloads"));

    QSqlQuery query("PRAGMA table_info(downloads)");
    QStringList columns;
    while (query.next()) {
        columns << query.value(1).toString();
    }

    EXPECT_TRUE(columns.contains("id"));
    EXPECT_TRUE(columns.contains("name"));
    EXPECT_TRUE(columns.contains("url"));
    EXPECT_TRUE(columns.contains("filePath"));
    EXPECT_TRUE(columns.contains("status"));
    EXPECT_TRUE(columns.contains("totalBytes"));
    EXPECT_TRUE(columns.contains("downloadedBytes"));
    EXPECT_TRUE(columns.contains("expectedHash"));
    EXPECT_TRUE(columns.contains("actualHash"));
    EXPECT_TRUE(columns.contains("hashAlgorithm"));
    EXPECT_TRUE(columns.contains("chunkHashes"));
    EXPECT_TRUE(columns.contains("createdAt"));
    EXPECT_TRUE(columns.contains("updatedAt"));
    EXPECT_EQ(columns.size(), 13);
}

TEST_F(DownloadDatabaseTest, SaveAndLoadFullRecord)
{
    QSignalSpy spy(db, &DownloadDatabase::saveSuccesed);

    DownloadRecord record;
    record.m_name = "file.zip";
    record.m_url = "https://test.com/file.zip";
    record.m_filePath = "/home/user/downloads/file.zip";
    record.m_status = "downloading";
    record.m_totalBytes = 1024 * 1024;
    record.m_downloadedBytes = 512;
    record.m_expectedHash = "5d41402abc4b2a76b9719d911017c592";
    record.m_actualHash = "5d41402abc4b2a76b9719d911017c592";
    record.m_hashAlgorithm = "MD5";
    record.m_chunkHashes = QByteArray("\x01\x02\x03\x04", 4);

    QVector<DownloadRecord> toSave = { record };
    db->saveDownloads(toSave);

    ASSERT_EQ(spy.count(), 1);

    QVector<DownloadRecord> loaded = db->getDownloads();

    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded[0].m_name, record.m_name);
    EXPECT_EQ(loaded[0].m_url, record.m_url);
    EXPECT_EQ(loaded[0].m_filePath, record.m_filePath);
    EXPECT_EQ(loaded[0].m_status, record.m_status);
    EXPECT_EQ(loaded[0].m_totalBytes, record.m_totalBytes);
    EXPECT_EQ(loaded[0].m_downloadedBytes, record.m_downloadedBytes);
    EXPECT_EQ(loaded[0].m_expectedHash, record.m_expectedHash);
    EXPECT_EQ(loaded[0].m_actualHash, record.m_actualHash);
    EXPECT_EQ(loaded[0].m_hashAlgorithm, record.m_hashAlgorithm);
    EXPECT_EQ(loaded[0].m_chunkHashes, record.m_chunkHashes);
}

TEST_F(DownloadDatabaseTest, UpsertPreventsDuplicates)
{
    DownloadRecord record;
    record.m_url = "https://test.com/file.zip";
    record.m_name = "file.zip";
    record.m_filePath = "/home/user/downloads/file.zip";
    record.m_status = "pending";
    record.m_totalBytes = 1048576;
    record.m_downloadedBytes = 0;
    record.m_expectedHash = "HASH_EXPECTED";
    record.m_actualHash = "HASH_OLD";
    record.m_hashAlgorithm = "MD5";
    record.m_chunkHashes = QByteArray("OLD_CHUNKS");

    db->saveDownloads({record});

    DownloadRecord updated = record;
    updated.m_name = "new_name.zip";
    updated.m_filePath = "/new/path/file.zip";
    updated.m_status = "completed";
    updated.m_totalBytes = 2000000;
    updated.m_downloadedBytes = 2000000;
    updated.m_expectedHash = "5d41402abc4b2a76b9719d911017c592";
    updated.m_actualHash = "5d41402abc4b2a76b9719d911017c592";
    updated.m_hashAlgorithm = "SHA-256";
    updated.m_chunkHashes = QByteArray("\x01\x02\x03\x04", 4);

    db->saveDownloads({updated});

    QVector<DownloadRecord> loaded = db->getDownloads();

    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded[0].m_name, updated.m_name);
    EXPECT_EQ(loaded[0].m_url, updated.m_url);
    EXPECT_EQ(loaded[0].m_filePath, updated.m_filePath);
    EXPECT_EQ(loaded[0].m_status, updated.m_status);
    EXPECT_EQ(loaded[0].m_totalBytes, updated.m_totalBytes);
    EXPECT_EQ(loaded[0].m_downloadedBytes, updated.m_downloadedBytes);
    EXPECT_EQ(loaded[0].m_expectedHash, updated.m_expectedHash);
    EXPECT_EQ(loaded[0].m_actualHash, updated.m_actualHash);
    EXPECT_EQ(loaded[0].m_hashAlgorithm, updated.m_hashAlgorithm);
    EXPECT_EQ(loaded[0].m_chunkHashes, updated.m_chunkHashes);

}

TEST_F(DownloadDatabaseTest, CorrectSortingByStatus)
{
    QStringList expectedOrder = {
        "downloading", "resumed", "resumed_in_downloading", "start_new_task",
        "resumed_in_pending", "pending", "prepared", "preparing",
        "paused", "paused_new", "paused_resume", "completed",
        "error", "cancelled", "deleted"
    };

    QVector<DownloadRecord> recordsToSave;
    for (int i = expectedOrder.size() - 1; i >= 0; --i) {
        DownloadRecord record;
        record.m_url = QString("https://example.com/%1").arg(i);
        record.m_name = QString("File_%1").arg(i);
        record.m_filePath = QString("/downloads/test_file_%1").arg(i);
        record.m_status = expectedOrder.at(i);
        recordsToSave.append(record);
    }

    db->saveDownloads(recordsToSave);

    QVector<DownloadRecord> loaded = db->getDownloads();

    ASSERT_EQ(loaded.size(), expectedOrder.size());

    for (int i = 0; i < expectedOrder.size(); ++i) {
        EXPECT_EQ(loaded.at(i).m_status, expectedOrder.at(i))
        << "Status mismatch at index " << i;
    }
}

TEST_F(DownloadDatabaseTest, DeleteDownload)
{
    DownloadRecord record;
    record.m_name = "test_file.zip";
    record.m_url = "https://example.com/file";
    record.m_filePath = "/downloads/test_file.zip";
    db->saveDownloads({record});

    db->deleteDownload(record);

    auto loaded = db->getDownloads();
    EXPECT_TRUE(loaded.isEmpty()) << "Record should be deleted from DB";
}

TEST_F(DownloadDatabaseTest, TimestampsAreAutoGenerated)
{
    DownloadRecord record;
    record.m_url = "https://time.test";
    record.m_name = "time_file";
    record.m_filePath = "/downloads/test_file.zip";
    record.m_status = "downloading";

    db->saveDownloads({record});

    QSqlQuery query("SELECT createdAt, updatedAt FROM downloads", QSqlDatabase::database());
    ASSERT_TRUE(query.next());

    EXPECT_FALSE(query.value(0).toString().isEmpty()) << "createdAt should not be empty";
    EXPECT_FALSE(query.value(1).toString().isEmpty()) << "updatedAt should not be empty";
}

TEST_F(DownloadDatabaseTest, SaveMixedValidAndInvalidRecords) {
    DownloadDatabase db(":memory:");
    QVector<DownloadRecord> records;

    for (int i = 0; i < 7; ++i) {
        DownloadRecord record;
        record.m_url = QString("https://example.com/file%1").arg(i);
        record.m_name = QString("File %1").arg(i);
        record.m_filePath = QString("/downloads/file%1.zip").arg(i);
        record.m_status = "pending";
        record.m_totalBytes = 1000;
        record.m_downloadedBytes = 0;
        records.append(record);
    }

    DownloadRecord badPath = records[0];
    badPath.m_url = "https://example.com/bad_path";
    badPath.m_filePath = "";
    records.append(badPath);

    DownloadRecord badBytes = records[0];
    badBytes.m_url = "https://example.com/bad_bytes";
    badBytes.m_totalBytes = -500;
    records.append(badBytes);

    DownloadRecord badName = records[0];
    badName.m_url = "https://example.com/bad_name";
    badName.m_name = "";
    records.append(badName);

    db.saveDownloads(records);

    QVector<DownloadRecord> loaded = db.getDownloads();

    EXPECT_EQ(loaded.size(), 7) << "Database should contain only valid records";

    for (const auto& record : loaded) {
        EXPECT_FALSE(record.m_url.contains("bad")) << "Invalid record found in DB: " << record.m_url.toStdString();
    }
}

void removeTestFile(const QString& path)
{
    if (QFile::exists(path)) QFile::remove(path);
}


TEST_F(DownloadDatabaseTest, HandlesEmptyPath)
{
    TearDown();
    db = new DownloadDatabase("");

    QString defaultPath = callGetDatabasePath();
    EXPECT_FALSE(defaultPath.isEmpty());
    EXPECT_TRUE(defaultPath.contains("download.db"));
}

TEST_F(DownloadDatabaseTest, CreatesMissingDirectories) {
    QString nestedPath = QDir::tempPath() + "/my_app/tests/db/test.db";
    QDir(QDir::tempPath() + "/my_app").removeRecursively();

    TearDown();
    db = new DownloadDatabase(nestedPath);

    QFileInfo checkFile(nestedPath);
    EXPECT_TRUE(checkFile.absoluteDir().exists()) << "Directories should be created automatically";

    removeTestFile(nestedPath);
}

TEST_F(DownloadDatabaseTest, RejectPathIfItIsDirectory) {
    QString dirPath = QDir::tempPath() + "/test_dir_as_db";
    QDir().mkpath(dirPath);

    EXPECT_FALSE(callIsValidPath(dirPath));

    QDir(dirPath).removeRecursively();
}

TEST_F(DownloadDatabaseTest, HandlesNoWritePermissions) {
#ifdef Q_OS_WIN
    QString winPath = "C:/Windows/system_db_test.db";
    //EXPECT_FALSE(callIsValidPath(winPath)) << "Should reject path in protected Windows directory";

    EXPECT_FALSE(callIsValidPath("Z:/non_existent_drive/test.db")) << "Should reject non-existent drive";
#endif

#ifdef Q_OS_UNIX
    EXPECT_FALSE(callIsValidPath("/protected_test.db")) << "Should reject path in root directory without permissions";

    EXPECT_FALSE(callIsValidPath("/usr/bin/test_db_logic.db")) << "Should reject path in system bin directory";
#endif

    QString validTempPath = QDir::tempPath() + "/writable_dir/test.db";
    QDir(QDir::tempPath() + "/writable_dir").removeRecursively();

    //EXPECT_TRUE(callIsValidPath(validTempPath)) << "Should allow path in writable temp directory";

    QString roDirPath = QDir::tempPath() + "/readonly_folder";
    QDir().mkpath(roDirPath);

#ifdef Q_OS_UNIX
    QFile::setPermissions(roDirPath, QFileDevice::ReadOwner | QFileDevice::ExeOwner);
#endif

    //EXPECT_FALSE(callIsValidPath(roDirPath + "/test.db")) << "Should reject path in Read-Only directory";

    QFile::setPermissions(roDirPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QDir(roDirPath).removeRecursively();
}

TEST_F(DownloadDatabaseTest, HandlesSpacesInPath) {
#ifdef Q_OS_WIN
    EXPECT_FALSE(callIsValidPath("test?.db")) << "Windows should reject '?'";
    EXPECT_FALSE(callIsValidPath("data*.db")) << "Windows should reject '*'";
    EXPECT_FALSE(callIsValidPath("my|file.db")) << "Windows should reject '|'";
    EXPECT_FALSE(callIsValidPath("quote\".db")) << "Windows should reject double quotes";

    EXPECT_FALSE(callIsValidPath("CON.db")) << "Windows should reject reserved name CON";
    EXPECT_FALSE(callIsValidPath("LPT1")) << "Windows should reject reserved name LPT1";
#endif

#ifdef Q_OS_MAC
    EXPECT_FALSE(callIsValidPath("my:file.db")) << "macOS should reject ':' in filename";
#endif

#ifdef Q_OS_UNIX
    EXPECT_TRUE(callIsValidPath("data?.db")) << "Linux should allow '?' in filename";
    EXPECT_TRUE(callIsValidPath("my|file.db")) << "Linux should allow '|' in filename";
#endif

    QString dirPath = QDir::tempPath() + "/test_directory";
    QDir().mkpath(dirPath);
    EXPECT_FALSE(callIsValidPath(dirPath)) << "Path to an existing directory should be invalid";
    QDir(dirPath).removeRecursively();
}
