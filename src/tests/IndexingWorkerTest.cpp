#include <catch2/catch_test_macros.hpp>
#include <QSignalSpy>
#include <QStringList>
#include "../IndexingWorker.h"
#include "../Library.h"
#include "../Model.h"
#include <QCoreApplication>
#include <QDir>
#include <filesystem>
#include <iostream>

// MockLibrary class for testing purposes without modifying Library.h
class MockLibrary : public Library {
public:
    MockLibrary(const char* label, const char* path)
        : Library(label, path) {}

    QStringList models;
};

void createTestDirectories() {
    QDir().mkdir("./temp_test_library");
    QDir().mkdir("./temp_test_library/.cadventory");
}

void removeTestDirectories() {
    QDir tempLibrary("./temp_test_library");
    tempLibrary.removeRecursively();
}

TEST_CASE("IndexingWorker - Initialization", "[IndexingWorker]") {
    createTestDirectories();
    Model model("./temp_test_library");
    MockLibrary library("Test Library", "./temp_test_library");
    IndexingWorker worker(&library);

    REQUIRE(worker.parent() == nullptr);
    REQUIRE(library.models.isEmpty());

    removeTestDirectories();
}

TEST_CASE("IndexingWorker - Process Method", "[IndexingWorker]") {
    createTestDirectories();
    Model model("./temp_test_library");
    MockLibrary library("Test Library", "./temp_test_library");

    // Add mock .g files for testing
    library.models = {"test1.g", "test2.g"};

    IndexingWorker worker(&library);
    QSignalSpy progressSpy(&worker, &IndexingWorker::progressUpdated);
    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished);

    worker.process();

    REQUIRE(progressSpy.count() > 0);
    REQUIRE(finishedSpy.count() == 1);

    auto lastProgress = progressSpy.takeLast();
    REQUIRE(lastProgress.at(0).toString() == "Processing complete");
    REQUIRE(lastProgress.at(1).toInt() == 100);

    removeTestDirectories();
}

TEST_CASE("IndexingWorker - Stop Functionality", "[IndexingWorker]") {
    createTestDirectories();
    Model model("./temp_test_library");
    MockLibrary library("Test Library", "./temp_test_library");

    library.models = {"test1.g", "test2.g"};

    IndexingWorker worker(&library);
    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished);

    worker.stop();
    worker.process();

    REQUIRE(finishedSpy.count() == 1);
    removeTestDirectories();
}

TEST_CASE("IndexingWorker - Full Processing of Files", "[IndexingWorker]") {
    createTestDirectories();
    Model model("./temp_test_library");
    MockLibrary library("Test Library", "./temp_test_library");

    // Create actual mock files
    QString testFile1 = "./temp_test_library/test1.g";
    QString testFile2 = "./temp_test_library/test2.g";
    
    QFile file1(testFile1);
    file1.open(QIODevice::WriteOnly);
    file1.write("Mock content for test1.g");
    file1.close();

    QFile file2(testFile2);
    file2.open(QIODevice::WriteOnly);
    file2.write("Mock content for test2.g");
    file2.close();

    library.models = {testFile1, testFile2};

    IndexingWorker worker(&library);
    QSignalSpy progressSpy(&worker, &IndexingWorker::progressUpdated);
    QSignalSpy modelProcessedSpy(&worker, &IndexingWorker::modelProcessed);
    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished);

    worker.process();

    REQUIRE(progressSpy.count() > 0);
    REQUIRE(modelProcessedSpy.count() == 2);
    REQUIRE(finishedSpy.count() == 1);

    removeTestDirectories();
}