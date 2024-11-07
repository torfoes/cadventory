#include <catch2/catch_test_macros.hpp>
#include <QSignalSpy>
#include <QStringList>
#include "../IndexingWorker.h"
#include "../Library.h"
#include "../Model.h"
#include <QCoreApplication>
#include <QDir>
#include <filesystem>

// MockLibrary class for testing purposes without modifying Library.h
class MockLibrary : public Library {
public:
    MockLibrary(const char* label, const char* path)
        : Library(label, path) {}

    // Instead of overriding getModels(), set models directly for testing
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