#ifndef CHUNKPROCESSOR_H
#define CHUNKPROCESSOR_H

#include <QObject>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>

class ChunkProcessor : public QObject
{
    Q_OBJECT
public:
    ChunkProcessor(QObject *parent = nullptr);
    void setChunkSize(qint64 size);
    void reset(int startChunkIndex = 0);
    qint64 getCurrentIndex() {return m_currentChunkIndex;};
    void setCryptographicAlgorithm(QCryptographicHash::Algorithm algoritm);
public slots:
    void processData(const QByteArray &data);
    void finalize();
signals:
    void chunkReady(int index, const QByteArray &data, const QByteArray &hash);
private:
    QCryptographicHash::Algorithm m_activeAlgorithm = QCryptographicHash::Sha256;
    QByteArray m_buffer;
    qint64 m_chunkSize{1024 * 1024};
    int m_currentChunkIndex{0};
};

#endif // CHUNKPROCESSOR_H
