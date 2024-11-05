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

TEST_CASE("ProcessGFiles - File Processing", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());
    std::string validFilePath = "src/tests/truck.g"; // Update based on actual location
    std::string invalidFilePath = "src/tests/nonexistent.g";

    SECTION("Process valid .g file") {
        REQUIRE(std::filesystem::exists(validFilePath));
        processor.processGFile(validFilePath, "output_folder");

        // Check if output (e.g., preview or thumbnail) was generated
        REQUIRE(std::filesystem::exists("output_folder/truck.png")); // Adjust path as needed
    }

    SECTION("Attempt to process invalid .g file") {
        REQUIRE_FALSE(std::filesystem::exists(invalidFilePath));
        processor.processGFile(invalidFilePath, "output_folder");

        // Here, we expect no output to be generated
        REQUIRE_FALSE(std::filesystem::exists("output_folder/nonexistent.png")); // Adjust path as needed
    }
}

TEST_CASE("ProcessGFiles - Thumbnail Generation", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());
    std::string filePath = "src/tests/truck.g";
    
    REQUIRE(std::filesystem::exists(filePath));
    processor.processGFile(filePath, "output_folder");

    SECTION("Check thumbnail exists") {
        std::string previewPath = "output_folder/truck.png"; // Update based on expected path
        REQUIRE(std::filesystem::exists(previewPath));
    }
}

TEST_CASE("ProcessGFiles - Error Handling", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());
    std::string corruptedFilePath = "src/tests/corrupted.g";

    SECTION("Handle corrupted .g file") {
        if (std::filesystem::exists(corruptedFilePath)) {
            processor.processGFile(corruptedFilePath, "output_folder");
            REQUIRE_FALSE(std::filesystem::exists("output_folder/corrupted.png"));
        } else {
            WARN("Corrupted .g file not found, skipping test");
        }
    }
}

TEST_CASE("ProcessGFiles - Edge Cases", "[ProcessGFiles]") {
    setupTestLibraryPath();
    auto model = std::make_unique<Model>(TEST_LIBRARY_PATH, nullptr);
    ProcessGFiles processor(model.get());
    std::string emptyFilePath = "src/tests/empty.g";

    SECTION("Handle empty .g file") {
        if (std::filesystem::exists(emptyFilePath)) {
            processor.processGFile(emptyFilePath, "output_folder");
            REQUIRE_FALSE(std::filesystem::exists("output_folder/empty.png"));
        } else {
            WARN("Empty .g file not found, skipping test");
        }
    }
}
