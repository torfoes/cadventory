#ifndef INDEXINGWORKER_H
#define INDEXINGWORKER_H

#include <QObject>
#include "Library.h"

class IndexingWorker : public QObject {
    Q_OBJECT
public:
    explicit IndexingWorker(Library* library, QObject* parent = nullptr);

public slots:
    void process();

signals:
    void modelProcessed(int modelId);
    void finished();

private:
    Library* library;
};

#endif // INDEXINGWORKER_H
