#include <catch2/catch_test_macros.hpp>
#include "../ProcessGFiles.h" 
#include "../Model.h"
#include <filesystem>
#include <memory>

const std::string TEST_LIBRARY_PATH = "./temp_test_library";  // Use a writable path

void setupTestLibraryPath() {
    // Ensure the base directory and .cadventory subdirectory exist
    std::filesystem::create_directories(TEST_LIBRARY_PATH + "/.cadventory");
}

TEST_CASE("ProcessGFiles - Initialization", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());

    REQUIRE(model != nullptr);
    REQUIRE(&processor != nullptr);
}

TEST_CASE("ProcessGFiles - File Processing with annual_gift_man.g", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());
    std::string validFilePath = "../src/tests/annual_gift_man.g"; // Update based on actual location

    SECTION("Process annual_gift_man.g file") {
        if (std::filesystem::exists(validFilePath)) {
            processor.processGFile(validFilePath, "output_folder");

            // Check if output (e.g., preview or thumbnail) was generated
            REQUIRE(std::filesystem::exists("output_folder/annual_gift_man.png")); // Adjust path as needed
        } else {
            WARN("annual_gift_man.g file not found, skipping test");
        }
    }
}

TEST_CASE("ProcessGFiles - Thumbnail Generation", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());
    std::string filePath = "../src/tests/annual_gift_man.g";
    
    REQUIRE(std::filesystem::exists(filePath));
    processor.processGFile(filePath, "output_folder");

    SECTION("Check thumbnail exists for annual_gift_man.g") {
        std::string previewPath = "output_folder/annual_gift_man.png"; // Update based on expected path
        REQUIRE(std::filesystem::exists(previewPath));
    }
}

// TEST_CASE("ProcessGFiles - Generate Gist Report for annual_gift_man.g", "[ProcessGFiles]") {
//     setupTestLibraryPath();
//     auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
//     ProcessGFiles processor(model.get());

//     std::string outputFilePath = "output_folder/annual_gist_report.txt";
//     auto [success, errorMessage] = processor.generateGistReport("src/tests/annual_gift_man.g", outputFilePath, "annual_gift_man");

//     // Check if report generation was successful
//     REQUIRE(success == true);
//     REQUIRE(errorMessage.empty());

//     // Confirm output file was created
//     REQUIRE(std::filesystem::exists(outputFilePath));
// }
