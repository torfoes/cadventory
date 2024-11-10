#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "Library.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

void createTestFiles(const std::string& directory, const std::vector<std::string>& filenames) {
    for (const auto& filename : filenames) {
        std::filesystem::path filePath = std::filesystem::path(directory) / filename;
        std::ofstream outfile(filePath);
        outfile << "Test content for " << filename << std::endl;
        outfile.close();
    }
}

void cleanupTestDirectory(const std::string& path) {
    if (std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
}

TEST_CASE("Library Operations", "[Library]") {
    std::string testDir = (std::filesystem::temp_directory_path() / "cadventory_library_test").string();
    cleanupTestDirectory(testDir); 
    std::filesystem::create_directories(testDir);

    std::vector<std::string> testFiles = {
        "model1.g",
        "model2.g",
        "geometry1.stl",
        "geometry2.obj",
        "image1.jpg",
        "image2.png",
        "document1.pdf",
        "document2.txt",
        "data1.csv",
        "data2.json",
        "otherfile.xyz"
    };

    createTestFiles(testDir, testFiles);

    Library library("TestLibrary", testDir.c_str());

    SECTION("Library Initialization") {
        REQUIRE(std::string(library.name()) == "TestLibrary");
        REQUIRE(std::string(library.path()) == testDir);
    }

    SECTION("Indexing Files") {
        size_t indexedFiles = library.indexFiles();
        REQUIRE(indexedFiles == testFiles.size() + 1);
    }

    SECTION("Get Models") {
        auto models = library.getModels();
        REQUIRE(models.size() == 2);

        std::vector<std::string> expectedModels = {"model1.g", "model2.g"};
        REQUIRE(std::is_permutation(models.begin(), models.end(), expectedModels.begin()));
    }

    SECTION("Get Geometry Files") {
        auto geometryFiles = library.getGeometry();
        
        std::vector<std::string> expectedGeometryFiles = {"model1.g", "model2.g", "geometry1.stl", "geometry2.obj"};
        
        std::vector<std::string> geometryFileBasenames;
        for (const auto& file : geometryFiles) {
            geometryFileBasenames.push_back(std::filesystem::path(file).filename().string());
        }
        
        REQUIRE(std::is_permutation(geometryFileBasenames.begin(), geometryFileBasenames.end(), expectedGeometryFiles.begin()));
    }

    SECTION("Get Image Files") {
        auto imageFiles = library.getImages();
        
        std::vector<std::string> expectedImageFiles = {"image1.jpg", "image2.png"};
        
        std::vector<std::string> imageFileBasenames;
        for (const auto& file : imageFiles) {
            imageFileBasenames.push_back(std::filesystem::path(file).filename().string());
        }
        
        REQUIRE(std::is_permutation(imageFileBasenames.begin(), imageFileBasenames.end(), expectedImageFiles.begin()));
    }

    SECTION("Get Document Files") {
        auto documentFiles = library.getDocuments();
        
        std::vector<std::string> expectedDocumentFiles = {"document1.pdf", "document2.txt"};
        
        std::vector<std::string> documentFileBasenames;
        for (const auto& file : documentFiles) {
            documentFileBasenames.push_back(std::filesystem::path(file).filename().string());
        }
        
        REQUIRE(std::is_permutation(documentFileBasenames.begin(), documentFileBasenames.end(), expectedDocumentFiles.begin()));
    }

    SECTION("Get Data Files") {
        auto dataFiles = library.getData();

        std::vector<std::string> expectedDataFiles = {"data1.csv", "data2.json", "otherfile.xyz"};

        // Extract only the basenames for comparison
        std::vector<std::string> dataFileBasenames;
        for (const auto& file : dataFiles) {
            dataFileBasenames.push_back(std::filesystem::path(file).filename().string());
        }

        REQUIRE(std::is_permutation(dataFileBasenames.begin(), dataFileBasenames.end(), expectedDataFiles.begin()));
    }

    SECTION("Load Database") {
        REQUIRE_NOTHROW(library.loadDatabase());
    }

    // Clean up after tests
    cleanupTestDirectory(testDir);
}