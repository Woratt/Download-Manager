#include <gtest/gtest.h>
#include <QtTest/QSignalSpy>
#include <QThread>
#include <QTimer>
#include "downloadregistry.h"

class DownloadRegistryTest : public ::testing::Test {
protected:
    DownloadRegistry *registry;

    void SetUp() override {
        registry = new DownloadRegistry();
    }

    void TearDown() override {
        delete registry;
    }

    DownloadTypes::DownloadRecord createFullRecord() {
        DownloadTypes::DownloadRecord record;
        record.id = QUuid::createUuid();
        record.name = "test_file.zip";
        record.url = "https://example.com/test.zip";
        record.filePath = "/downloads/test_file.zip";
        record.expectedHash = "e4d909c290d0fb1ca068ffaddf22cbd0";
        record.actualHash = "";
        record.hashAlgorithm = "MD5";
        record.chunkHashes = {QByteArray("chunk1"), QByteArray("chunk2")};
        record.status = DownloadTypes::DownloadStatus::Preparing;
        record.totalBytes = 1048576;
        record.downloadedBytes = 0;
        record.quantityOfChunks = 8;
        return record;
    }
};

TEST_F(DownloadRegistryTest, AddRecordCreatesUniqueIdAndStoresData){
    auto input = createFullRecord();
    QUuid generatedId = registry->addRecord(std::move(input));

    ASSERT_FALSE(generatedId.isNull()) << "ID не згенеровано";

    auto record = registry->getRecord(generatedId);
    EXPECT_EQ(record.name, QString("test_file.zip"));
    EXPECT_EQ(record.url, QString("https://example.com/test.zip"));
    EXPECT_EQ(record.filePath, QString("/downloads/test_file.zip"));
    EXPECT_EQ(record.expectedHash, QString("e4d909c290d0fb1ca068ffaddf22cbd0"));
    EXPECT_EQ(record.actualHash, QString(""));
    EXPECT_EQ(record.hashAlgorithm, QString("MD5"));
    EXPECT_EQ(record.chunkHashes.size(), 2);
    EXPECT_EQ(record.status, DownloadTypes::DownloadStatus::Preparing);
    EXPECT_EQ(record.totalBytes, qint64(1048576));
    EXPECT_EQ(record.downloadedBytes, qint64(0));
    EXPECT_EQ(record.quantityOfChunks, qint64(8));
}

TEST_F(DownloadRegistryTest, UpdateProgressEmitsSignalAndChangesBytes) {
    auto input = createFullRecord();
    QUuid id = registry->addRecord(std::move(input));

    QSignalSpy spy(registry, &DownloadRegistry::progressChanged);
    registry->updateProgress(id, 524288, 1048576);

    ASSERT_EQ(spy.count(), 1);
    auto args = spy.takeFirst();
    EXPECT_EQ(args[0].value<QUuid>(), id);
    EXPECT_EQ(args[1].value<qint64>(), qint64(524288));
    EXPECT_EQ(args[2].value<qint64>(), qint64(1048576));

    auto record = registry->getRecord(id);
    EXPECT_EQ(record.downloadedBytes, qint64(524288));
    EXPECT_EQ(record.totalBytes, qint64(1048576));
}

TEST_F(DownloadRegistryTest, UpdateStatusEmitsSignalAndChangesStatus) {
    auto input = createFullRecord();
    QUuid id = registry->addRecord(std::move(input));

    QSignalSpy spy(registry, &DownloadRegistry::statusChanged);
    registry->updateStatus(id, DownloadTypes::DownloadStatus::Downloading);

    ASSERT_EQ(spy.count(), 1);
    auto args = spy.takeFirst();
    EXPECT_EQ(args[0].value<QUuid>(), id);
    EXPECT_EQ(args[1].value<DownloadTypes::DownloadStatus>(), DownloadTypes::DownloadStatus::Downloading);

    auto record = registry->getRecord(id);
    EXPECT_EQ(record.status, DownloadTypes::DownloadStatus::Downloading);
}

TEST_F(DownloadRegistryTest, GetNonExistentRecordReturnsDefaultConstructed) {
    QUuid fakeId = QUuid::createUuid();
    auto record = registry->getRecord(fakeId);
    EXPECT_TRUE(record.id.isNull());
    EXPECT_TRUE(record.name.isEmpty());
    EXPECT_TRUE(record.url.isEmpty());
    EXPECT_EQ(record.totalBytes, qint64(0));
    EXPECT_EQ(record.downloadedBytes, qint64(0));
    EXPECT_EQ(record.status, DownloadTypes::DownloadStatus::Preparing);
    EXPECT_EQ(record.quantityOfChunks, qint64(8));
}

TEST_F(DownloadRegistryTest, GetAllRecordsReturnsAllAdded) {
    registry->addRecord(DownloadTypes::DownloadRecord{QUuid::createUuid(),"", "url1", "path1"});
    registry->addRecord(DownloadTypes::DownloadRecord{QUuid::createUuid(),"", "url2", "path2"});

    auto all = registry->getAllRecords();
    ASSERT_EQ(all.size(), 2);

    QStringList urls;
    for(const auto& rec : all) {
        urls << rec.url;
    }

    EXPECT_TRUE(urls.contains("url1"));
    EXPECT_TRUE(urls.contains("url2"));
}

TEST_F(DownloadRegistryTest, AddRecordUsesMoveSemantics) {
    DownloadTypes::DownloadRecord record;
    record.name = "moved_file";
    record.chunkHashes = {QByteArray("chunk")};

    registry->addRecord(std::move(record));

    EXPECT_TRUE(record.name.isEmpty());
    EXPECT_TRUE(record.chunkHashes.isEmpty());
}

TEST_F(DownloadRegistryTest, ThreadSafetyWithRealThreads) {
    auto record = createFullRecord();
    QUuid id = registry->addRecord(std::move(record));

    constexpr int numThreads = 5;
    constexpr int updatesPerThread = 50;

    QVector<QThread*> threads;
    for (int t = 0; t < numThreads; ++t) {
        QThread* thread = new QThread();

        QObject::connect(thread, &QThread::started, [this, id, thread, updatesPerThread]() {
            for (int i = 0; i < updatesPerThread; ++i) {
                registry->updateProgress(id, (i + 1) * 10, 1000);
                registry->updateStatus(id, static_cast<DownloadTypes::DownloadStatus>((i % 8) + 1));
                QThread::msleep(1);
            }
            thread->quit();
        });

        threads.append(thread);
        thread->start();
    }

    for (QThread* t : threads) {
        while (t->isRunning()) {
            QCoreApplication::processEvents();
            QThread::msleep(10);
        }
        t->deleteLater();
    }

    auto finalRec = registry->getRecord(id);
    EXPECT_EQ(finalRec.downloadedBytes, updatesPerThread * 10);
}
