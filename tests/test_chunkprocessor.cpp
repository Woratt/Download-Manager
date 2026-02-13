#include <gtest/gtest.h>
#include <QtTest/QSignalSpy>
#include "chunkprocessor.h"

class ChunkProcessorTest : public ::testing::Test {
protected:
    ChunkProcessor processor;

    void SetUp() override {
        processor.setChunkSize(10);
        processor.setCryptographicAlgorithm(QCryptographicHash::Md5);
        processor.reset(0);
    }

};

TEST_F(ChunkProcessorTest, ProcessExactChunkSize){
    QSignalSpy spy(&processor, &ChunkProcessor::chunkReady);

    QByteArray data("12345678901234567890");
    processor.processData(data);

    ASSERT_EQ(spy.count(), 2);

    QList<QVariant> firstSignal = spy.at(0);
    EXPECT_EQ(firstSignal.at(0).toInt(), 0);
    EXPECT_EQ(firstSignal.at(1).toByteArray().size(), 10);
}

TEST_F(ChunkProcessorTest, FinalizeHandlesRemainder){
    QSignalSpy spy(&processor, &ChunkProcessor::chunkReady);

    QByteArray data("12345");
    processor.processData(data);

    EXPECT_EQ(spy.count(), 0);

    processor.finalize();

    ASSERT_EQ(spy.count(), 1);

    EXPECT_EQ(spy.at(0).at(1).toByteArray(), "12345");
}

TEST_F(ChunkProcessorTest, CorrectHashCalculation) {
    QSignalSpy spy(&processor, &ChunkProcessor::chunkReady);

    QByteArray data("1234567890");
    QByteArray expectedHash = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();

    processor.processData(data);

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(2).toByteArray(), expectedHash);
}

TEST_F(ChunkProcessorTest, ResetClearsStateCorrectly){
    QSignalSpy spy(&processor, &ChunkProcessor::chunkReady);

    processor.processData("12345");

    processor.reset(100);

    processor.processData("ABCDEFGHIJ");

    ASSERT_EQ(spy.count(), 1);

    EXPECT_EQ(spy.at(0).at(0).toInt(), 100);
    EXPECT_EQ(spy.at(0).at(1).toByteArray(), "ABCDEFGHIJ");
}

TEST_F(ChunkProcessorTest, AlgorithmChangeProducesDifferentHashes){
    QByteArray data = "test_data_10";

    QSignalSpy spyMd5(&processor, &ChunkProcessor::chunkReady);
    processor.processData(data);
    QByteArray hashMd5 = spyMd5.at(0).at(2).toByteArray();

    processor.reset(0);

    processor.setCryptographicAlgorithm(QCryptographicHash::Sha256);
    QSignalSpy spySha(&processor, &ChunkProcessor::chunkReady);
    processor.processData(data);
    QByteArray hashSha = spySha.at(0).at(2).toByteArray();

    EXPECT_NE(hashMd5, hashSha);
    EXPECT_EQ(hashSha.length(), 64);
}

TEST_F(ChunkProcessorTest, EmitsSignalWhenBufferIsFullAndIncrementsIndex) {
    QSignalSpy spy(&processor, &ChunkProcessor::chunkReady);

    processor.processData("12345678901234567890ABCDE");

    ASSERT_EQ(spy.count(), 2);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 0);
    EXPECT_EQ(spy.at(1).at(0).toInt(), 1);

    processor.finalize();
    ASSERT_EQ(spy.count(), 3);
    EXPECT_EQ(spy.at(2).at(1).toByteArray(), "ABCDE");
}

TEST_F(ChunkProcessorTest, FinalizeHandlesRemainingData) {
    QSignalSpy spy(&processor, &ChunkProcessor::chunkReady);

    processor.processData("small");
    EXPECT_EQ(spy.count(), 0);

    processor.finalize();

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(1).toByteArray(), "small");

    processor.finalize();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ChunkProcessorTest, IgnorEmptyData){
    QSignalSpy spy(&processor, &ChunkProcessor::chunkReady);

    processor.processData("");

    EXPECT_EQ(spy.count(), 0);
}

