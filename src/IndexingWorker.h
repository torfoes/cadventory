#ifndef INDEXINGWORKER_H
#define INDEXINGWORKER_H

#include <QObject>
#include <atomic>
#include "Library.h"

class IndexingWorker : public QObject {
    Q_OBJECT
public:
    explicit IndexingWorker(Library* library, QObject* parent = nullptr);
    void stop();

public slots:
    void process();

signals:
    void modelProcessed(int modelId);
    void finished();

private:
    Library* library;
    std::atomic<bool> m_stopRequested;
};

#endif // INDEXINGWORKER_H
