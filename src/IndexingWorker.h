#ifndef INDEXINGWORKER_H
#define INDEXINGWORKER_H

#include <QObject>
#include <atomic>
#include "Library.h"

class IndexingWorker : public QObject {
    Q_OBJECT
public:
    explicit IndexingWorker(Library* library,bool preview,QObject* parent = nullptr);
    void stop();

public slots:
    void process();

signals:
    void modelProcessed(int modelId);
    void progressUpdated(const QString& currentObject, int percentage);
    void finished();

private:
    Library* library;
    std::atomic<bool> m_stopRequested;
    bool previewFlag;
};

#endif // INDEXINGWORKER_H
