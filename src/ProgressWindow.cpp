#include "ProgressWindow.h"

// Constructor implementation
ProgressWindow::ProgressWindow(int totalFiles, QWidget *parent) : QWidget(parent), totalFiles(totalFiles), filesProcessed(0) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    progressBar = new QProgressBar(this);
    progressBar->setRange(0, totalFiles);
    progressBar->setValue(0);

    label = new QLabel(this);
    label->setText(QString("Processed 0 of %1 files").arg(totalFiles));

    layout->addWidget(label);
    layout->addWidget(progressBar);
    setLayout(layout);

    setWindowTitle("Processing .g Files");
    setMinimumSize(300, 100);
}

// Slot to update the progress
void ProgressWindow::updateProgress() {
    filesProcessed++;
    progressBar->setValue(filesProcessed);
    label->setText(QString("Processed %1 of %2 files").arg(filesProcessed).arg(totalFiles));

    // Close the window when all files are processed
    if (filesProcessed == totalFiles) {
        close();
    }
}
