#include "../headers/chunkprocessor.h"

ChunkProcessor::ChunkProcessor(QObject *parent) : QObject(parent) {}

void ChunkProcessor::setChunkSize(qint64 size)
{
    if (size > 0) m_chunkSize = size;
}

void ChunkProcessor::setCryptographicAlgorithm(QCryptographicHash::Algorithm algoritm){
    m_activeAlgorithm = algoritm;
}

void ChunkProcessor::reset(int startChunkIndex)
{
    m_buffer.clear();
    m_currentChunkIndex = startChunkIndex;
}

void ChunkProcessor::processData(const QByteArray &data)
{
    m_buffer.append(data);
    while (m_buffer.size() >= m_chunkSize) {
        QByteArray chunkData = m_buffer.left(m_chunkSize);
        m_buffer.remove(0, m_chunkSize);

        QByteArray hash = QCryptographicHash::hash(chunkData, m_activeAlgorithm).toHex();

        emit chunkReady(m_currentChunkIndex, chunkData, hash);
        m_currentChunkIndex++;
    }
}

void ChunkProcessor::finalize() {
    if (!m_buffer.isEmpty()) {
        QByteArray hash = QCryptographicHash::hash(m_buffer, m_activeAlgorithm).toHex();
        emit chunkReady(m_currentChunkIndex, m_buffer, hash);
        m_buffer.clear();
    }
}
