#include <catch2/catch_test_macros.hpp>
#include <QSignalSpy>
#include <QStringList>
#include "../IndexingWorker.h"
#include "../Library.h"
#include "../Model.h"
#include <QCoreApplication>
#include <QDir>
#include <filesystem>

// MockLibrary class for testing purposes
class MockLibrary : public Library {
public:
    MockLibrary(const char* label, const char* path)
        : Library(label, path) {}

    QStringList models;
};

// Helper function to set up test environment
void createTestDirectories() {
    QDir().mkdir("./temp_test_library");
    QDir().mkdir("./temp_test_library/.cadventory");
}

// Helper function to clean up test environment
void removeTestDirectories() {
    QDir tempLibrary("./temp_test_library");
    tempLibrary.removeRecursively();
}

// Test for initialization with an empty library
TEST_CASE("IndexingWorker - Initialization with Empty Library", "[IndexingWorker]") {
    createTestDirectories();
    MockLibrary library("Test Library", "./temp_test_library");
    IndexingWorker worker(&library);

    REQUIRE(worker.parent() == nullptr);
    REQUIRE(library.models.isEmpty());

    removeTestDirectories();
}

// Test stop functionality
TEST_CASE("IndexingWorker - Stop Functionality", "[IndexingWorker]") {
    createTestDirectories();
    MockLibrary library("Test Library", "./temp_test_library");
    IndexingWorker worker(&library);

    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished);
    worker.stop();
    worker.process();

    REQUIRE(finishedSpy.count() == 1);

    removeTestDirectories();
}

// Test handling of reindex request
TEST_CASE("IndexingWorker - Reindex Request Handling", "[IndexingWorker]") {
    createTestDirectories();
    MockLibrary library("Test Library", "./temp_test_library");

    IndexingWorker worker(&library);
    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished);

    worker.requestReindex();
    worker.process();

    REQUIRE(finishedSpy.count() == 1);

    removeTestDirectories();
}

// Test processing with an empty models list
TEST_CASE("IndexingWorker - Empty Models List", "[IndexingWorker]") {
    createTestDirectories();
    MockLibrary library("Test Library", "./temp_test_library");
    IndexingWorker worker(&library);

    QSignalSpy progressSpy(&worker, &IndexingWorker::progressUpdated);
    worker.process();

    REQUIRE(progressSpy.count() == 1); // Only one update: "No files to process"

    removeTestDirectories();
}

// Test file access errors
TEST_CASE("IndexingWorker - File Access Errors", "[IndexingWorker]") {
    createTestDirectories();
    MockLibrary library("Test Library", "./temp_test_library");
    library.models = {"missing_file.g"};

    IndexingWorker worker(&library);
    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished);
    worker.process();

    REQUIRE(finishedSpy.count() == 1);

    removeTestDirectories();
}

// Test processing a large number of files
TEST_CASE("IndexingWorker - Large Number of Files", "[IndexingWorker]") {
    createTestDirectories();
    MockLibrary library("Test Library", "./temp_test_library");

    // Simulate 100 files
    for (int i = 0; i < 100; ++i) {
        library.models.append(QString("file_%1.g").arg(i));
    }

    IndexingWorker worker(&library);
    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished);
    worker.process();

    REQUIRE(finishedSpy.count() == 1);

    removeTestDirectories();
}

// Test signal emission order
TEST_CASE("IndexingWorker - Signal Emission Order", "[IndexingWorker]") {
    createTestDirectories();
    MockLibrary library("Test Library", "./temp_test_library");

    library.models = {"test1.g", "test2.g"};
    IndexingWorker worker(&library);

    QSignalSpy progressSpy(&worker, &IndexingWorker::progressUpdated);
    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished);

    worker.process();

    REQUIRE(finishedSpy.count() == 1);
    REQUIRE(progressSpy.count() > 0);

    bool foundProcessingComplete = false;
    for (int i = 0; i < progressSpy.count(); ++i) {
        QString progress = progressSpy.at(i).at(0).toString();
        qDebug() << "Progress Update:" << progress;
        if (progress == "Processing complete") {
            foundProcessingComplete = true;
        }
    }

    REQUIRE(foundProcessingComplete);  // Verify "Processing complete" was emitted

    removeTestDirectories();
}