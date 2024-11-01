#ifndef ReportGeneratorWorker_H
#define ReportGeneratorWorker_H

#include <QString>
#include <QObject>
#include <atomic>
#include "Model.h"
#include <iostream>

class ReportGeneratorWorker : public QObject {
    Q_OBJECT
public:
    explicit ReportGeneratorWorker(/*Model* model = nullptr, QString output_directory = "", */QObject* parent = nullptr);
    void stop();
public slots:
    void process();

signals:
    void successfulGistCall(const QString& path_gist_output);
    void failedGistCall(const QString& filepath, const QString& errorMessage);
    void finishedReport();
    void finished();

private:
    QString output_directory;
    Model* model;
    std::atomic<bool> m_stopRequested;
};

#endif // ReportGeneratorWorker_H
