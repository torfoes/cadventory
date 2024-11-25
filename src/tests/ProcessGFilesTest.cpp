#include <catch2/catch_test_macros.hpp>
#include "../ProcessGFiles.h" 
#include "../Model.h"
#include <filesystem>
#include <memory>
#include <QDir>

const std::string TEST_LIBRARY_PATH = "./temp_test_library";  // Base path for testing

void setupTestLibraryPath() {
    std::filesystem::create_directories(TEST_LIBRARY_PATH + "/.cadventory");
}

// Test initialization and object creation
TEST_CASE("ProcessGFiles - Initialization", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());

    REQUIRE(model != nullptr);
    REQUIRE(&processor != nullptr);
}

// Test processing a file (using `annual_gift_man.g`)
TEST_CASE("ProcessGFiles - File Processing with annual_gift_man.g", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());
    std::string validFilePath = "../src/tests/annual_gift_man.g"; // Adjust to the correct path

    SECTION("Process annual_gift_man.g file") {
        if (std::filesystem::exists(validFilePath)) {
            processor.processGFile(validFilePath, "output_folder", "(unknown lib)");

            // Verify that the thumbnail was generated as expected
            REQUIRE(std::filesystem::exists("output_folder/annual_gift_man.png"));
        } else {
            WARN("annual_gift_man.g file not found, skipping test");
        }
    }
}

Test thumbnail generation for `annual_gift_man.g`
TEST_CASE("ProcessGFiles - Thumbnail Generation", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());
    std::string filePath = "../src/tests/annual_gift_man.g";

    REQUIRE(std::filesystem::exists(filePath));
    processor.processGFile(filePath, "output_folder", "(unknown lib)");

    if (std::filesystem::exists(filePath)) {
        processor.processGFile(filePath, "output_folder");

        SECTION("Check if thumbnail exists for annual_gift_man.g") {
            std::string previewPath = "output_folder/annual_gift_man.png";
            REQUIRE(std::filesystem::exists(previewPath));
        }
    } else {
        WARN("annual_gift_man.g file not found, skipping test");
    }
}


TEST_CASE("ProcessGFiles - Generate Gist Report for annual_gift_man.g", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());

    std::string outputFilePath = "output_folder/annual_gist_report.txt";
    auto [success, errorMessage] = processor.generateGistReport("../src/tests/annual_gift_man.g", outputFilePath, "annual_gift_man");

    REQUIRE(success == true);
    REQUIRE(errorMessage.empty());
    REQUIRE(std::filesystem::exists(outputFilePath));
}

