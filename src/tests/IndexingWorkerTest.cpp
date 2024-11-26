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

// MockLibrary class for testing purposes without modifying the original Library.h
class MockLibrary : public Library {
public:
    MockLibrary(const char* label, const char* path)
        : Library(label, path) {}

    QStringList models; // Mock list of model file names
};

// Helper function to create temporary test directories
void createTestDirectories() {
    QDir().mkdir("./temp_test_library");
    QDir().mkdir("./temp_test_library/.cadventory");
}

// Helper function to remove temporary test directories
void removeTestDirectories() {
    QDir tempLibrary("./temp_test_library");
    tempLibrary.removeRecursively();
}

// Test case for verifying the initialization of IndexingWorker
TEST_CASE("IndexingWorker - Initialization", "[IndexingWorker]") {
    createTestDirectories();
    Model model("./temp_test_library");
    MockLibrary library("Test Library", "./temp_test_library");
    IndexingWorker worker(&library);

    // Ensure the worker has no parent and the library's model list is empty
    REQUIRE(worker.parent() == nullptr);
    REQUIRE(library.models.isEmpty());

    removeTestDirectories();
}

// Test case for the process method of IndexingWorker
TEST_CASE("IndexingWorker - Process Method", "[IndexingWorker]") {
    createTestDirectories();
    Model model("./temp_test_library");
    MockLibrary library("Test Library", "./temp_test_library");

    // Add mock .g files to the library's model list
    library.models = {"test1.g", "test2.g"};

    IndexingWorker worker(&library);
    QSignalSpy progressSpy(&worker, &IndexingWorker::progressUpdated); // Monitor progress updates
    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished);       // Monitor completion signal

    worker.process();

    // Verify progress updates and ensure the process finishes
    REQUIRE(progressSpy.count() > 0);
    REQUIRE(finishedSpy.count() == 1);

    // Check the last progress update contains the correct information
    auto lastProgress = progressSpy.takeLast();
    REQUIRE(lastProgress.at(0).toString() == "Processing complete");
    REQUIRE(lastProgress.at(1).toInt() == 100);

    removeTestDirectories();
}

// Test case for stopping IndexingWorker before processing completes
TEST_CASE("IndexingWorker - Stop Functionality", "[IndexingWorker]") {
    createTestDirectories();
    Model model("./temp_test_library");
    MockLibrary library("Test Library", "./temp_test_library");

    // Add mock .g files to the library's model list
    library.models = {"test1.g", "test2.g"};

    IndexingWorker worker(&library);
    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished); // Monitor completion signal

    worker.stop();  // Stop the worker
    worker.process(); // Attempt to process after stopping

    // Verify the worker still emits a finished signal
    REQUIRE(finishedSpy.count() == 1);

    removeTestDirectories();
}

// Test case for full processing of files by IndexingWorker
TEST_CASE("IndexingWorker - Full Processing of Files", "[IndexingWorker]") {
    createTestDirectories();
    Model model("./temp_test_library");
    MockLibrary library("Test Library", "./temp_test_library");

    // Create mock files for testing
    QString testFile1 = "./temp_test_library/test1.g";
    QString testFile2 = "./temp_test_library/test2.g";

    // Write dummy content to mock files
    QFile file1(testFile1);
    file1.open(QIODevice::WriteOnly);
    file1.write("Mock content for test1.g");
    file1.close();

    QFile file2(testFile2);
    file2.open(QIODevice::WriteOnly);
    file2.write("Mock content for test2.g");
    file2.close();

    library.models = {testFile1, testFile2}; // Add files to the library's model list

    IndexingWorker worker(&library);
    QSignalSpy progressSpy(&worker, &IndexingWorker::progressUpdated); // Monitor progress updates
    QSignalSpy modelProcessedSpy(&worker, &IndexingWorker::modelProcessed); // Monitor model processing
    QSignalSpy finishedSpy(&worker, &IndexingWorker::finished); // Monitor completion signal

    worker.process();

    // Verify all expected signals were emitted
    REQUIRE(progressSpy.count() > 0);
    REQUIRE(modelProcessedSpy.count() == 2); // Two files processed
    REQUIRE(finishedSpy.count() == 1);

    removeTestDirectories();
}