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

