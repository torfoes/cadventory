
// /* let catch provide main() */
// #define CATCH_CONFIG_MAIN
// #include <catch2/catch_test_macros.hpp>

// #include "Model.h"
// #include <filesystem>


// void setupTestDB(const std::string& path) {
//   if (std::filesystem::exists(path)) {
//     std::filesystem::remove(path);
//   }
// }

// int createTestModel(Model& model, const std::string& name = "TestModel", const std::string& cadFile = "./truck.g", const std::string& overrideInfo = "{}") {
//   model.insertModel(cadFile, name, "primary", overrideInfo);
//   auto models = model.getModels();
//   if (!models.empty()) {
//     return models.front().id;
//   }
//   return -1; // oops.
// }


// TEST_CASE("ModelOperations", "[Model]") {
//   std::string testDB = "test_models.db";
//   setupTestDB(testDB);
//   Model model(testDB);

//   SECTION("Insert model") {
//     bool insertResult = model.insertModel("path/to/cad/file", "testModel", "primary", "{\"test\":true}");
//     REQUIRE(insertResult == true);
//   }

//   SECTION("Read model") {
//     int modelId = createTestModel(model);
//     REQUIRE(modelId != -1);

//     auto models = model.getModels();
//     REQUIRE(models.size() == 1);
//     REQUIRE(models.front().id == modelId);
//   }

//   SECTION("Update model") {
//     int modelId = createTestModel(model);
//     REQUIRE(modelId != -1);

//     bool updateResult = model.updateModel(modelId, "updatedModel", "new/path/to/cad/file", "{\"updated\":true}");
//     REQUIRE(updateResult == true);

//     auto models = model.getModels();
//     REQUIRE(models.size() == 1);
//     REQUIRE(models.front().short_name == "updatedModel");
//     REQUIRE(models.front().primary_file == "new/path/to/cad/file");
//     REQUIRE(models.front().override_info == "{\"updated\":true}");
//   }

//   SECTION("Delete model") {
//     int modelId = createTestModel(model);
//     REQUIRE(modelId != -1);

//     bool deleteResult = model.deleteModel(modelId);
//     REQUIRE(deleteResult == true);

//     auto models = model.getModels();
//     REQUIRE(models.empty() == true);
//   }

//   setupTestDB(testDB);
// }

// TEST_CASE("TagOperations", "[Model]") {
//   std::string testDB = "test_tags.db";
//   setupTestDB(testDB);
//   Model model(testDB);

//   SECTION("Add tag to model") {
//     int modelId = createTestModel(model);
//     REQUIRE(modelId != -1);

//     bool addResult = model.addTagToModel(modelId, "testTag");
//     REQUIRE(addResult == true);

//     auto tags = model.getTagsForModel(modelId);
//     REQUIRE(tags.size() == 1);
//     REQUIRE(tags.front() == "testTag");
//   }

//   SECTION("Remove tag from model") {
//     int modelId = createTestModel(model);
//     REQUIRE(modelId != -1);

//     model.addTagToModel(modelId, "testTag");
//     auto tags = model.getTagsForModel(modelId);
//     REQUIRE(tags.size() == 1);

//     bool removeResult = model.removeTagFromModel(modelId, "testTag");
//     REQUIRE(removeResult == true);

//     tags = model.getTagsForModel(modelId);
//     REQUIRE(tags.empty() == true);
//   }

//   SECTION("Get all tags from a model") {
//     int modelId = createTestModel(model);
//     REQUIRE(modelId != -1);

//     model.addTagToModel(modelId, "tag1");
//     model.addTagToModel(modelId, "tag2");
//     model.addTagToModel(modelId, "tag3");

//     auto tags = model.getTagsForModel(modelId);
//     REQUIRE(tags.size() == 3);
//     REQUIRE(tags[0] == "tag1");
//     REQUIRE(tags[1] == "tag2");
//     REQUIRE(tags[2] == "tag3");
//   }

//   setupTestDB(testDB);
// }
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "Model.h"
#include <filesystem>
#include <iostream>

// Helper function for testing directory
std::string setupTestDirectory() {
    auto tempDir = std::filesystem::temp_directory_path() / "cadventory_test";
    if (!std::filesystem::exists(tempDir)) {
        std::filesystem::create_directories(tempDir);
    }
    return tempDir.string();
}

// Cleanup function for the test directory
void cleanupTestDirectory(const std::string& path) {
    if (std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
}

TEST_CASE("Model Operations", "[Model]") {
    std::string testDir = setupTestDirectory();
    cleanupTestDirectory(testDir);  // Clean before starting

    // Re-create the directory for test purposes
    std::filesystem::create_directories(testDir);

    // Now initialize the Model with the test directory
    Model model(testDir);

    SECTION("Database Initialization") {
        // Ensure the hidden directory and database file exist
        REQUIRE(std::filesystem::exists(model.getHiddenDirectoryPath()));
        REQUIRE(std::filesystem::exists(testDir + "/.cadventory/metadata.db"));
    }

    SECTION("Insert and verify model") {
        ModelData newModel {1, "TestModel", "./path/to/cad/file", "{}", "Test Title", {}, "Author", "/file/path", "Library", true, false};
        REQUIRE(model.insertModel(newModel.id, newModel) == true);

        auto fetchedModel = model.getModelById(newModel.id);
        REQUIRE(fetchedModel.short_name == "TestModel");
        REQUIRE(fetchedModel.file_path == "/file/path");
        REQUIRE(fetchedModel.library_name == "Library");
        REQUIRE(fetchedModel.is_selected == true);
    }

    SECTION("Update model attributes") {
        ModelData updatedModel {1, "UpdatedModel", "./new/path", "{\"updated\":true}", "Updated Title", {}, "New Author", "/new/file/path", "NewLibrary", false, false};
        REQUIRE(model.insertModel(updatedModel.id, updatedModel) == true);

        updatedModel.short_name = "ModifiedModel";
        updatedModel.library_name = "ModifiedLibrary";
        REQUIRE(model.updateModel(updatedModel.id, updatedModel) == true);

        auto fetchedModel = model.getModelById(updatedModel.id);
        REQUIRE(fetchedModel.short_name == "ModifiedModel");
        REQUIRE(fetchedModel.library_name == "ModifiedLibrary");
        REQUIRE(fetchedModel.is_selected == false);
    }

    SECTION("Delete model") {
        ModelData deleteModel {2, "DeleteModel", "./delete/path", "{}", "Delete Title", {}, "Delete Author", "/delete/file/path", "DeleteLibrary", false, false};
        REQUIRE(model.insertModel(deleteModel.id, deleteModel) == true);
        REQUIRE(model.modelExists(deleteModel.id) == true);

        REQUIRE(model.deleteModel(deleteModel.id) == true);
        REQUIRE(model.modelExists(deleteModel.id) == false);
    }

    SECTION("Add and retrieve objects for model") {
        ModelData objModel {3, "ObjModel", "./obj/path", "{}", "Obj Title", {}, "Obj Author", "/obj/file/path", "ObjLibrary", false, false};
        REQUIRE(model.insertModel(objModel.id, objModel) == true);

        ObjectData obj1 {0, objModel.id, "Object1", -1, false};
        int obj1_id = model.insertObject(obj1);
        REQUIRE(obj1_id != -1);

        ObjectData obj2 {0, objModel.id, "Object2", obj1_id, true};
        int obj2_id = model.insertObject(obj2);
        REQUIRE(obj2_id != -1);

        auto objects = model.getObjectsForModel(objModel.id);
        REQUIRE(objects.size() == 2);
        REQUIRE(objects[0].name == "Object1");
        REQUIRE(objects[1].name == "Object2");
        REQUIRE(objects[1].parent_object_id == obj1_id);
        REQUIRE(objects[1].is_selected == true);
    }

    SECTION("Update and select objects") {
        ModelData objModel {4, "UpdateObjModel", "./update/path", "{}", "Update Obj Title", {}, "Update Obj Author", "/update/file/path", "UpdateLibrary", false, false};
        REQUIRE(model.insertModel(objModel.id, objModel) == true);

        ObjectData obj {0, objModel.id, "SelectableObject", -1, false};
        int obj_id = model.insertObject(obj);
        REQUIRE(obj_id != -1);

        REQUIRE(model.updateObjectSelection(obj_id, true) == true);
        auto updatedObj = model.getObjectById(obj_id);
        REQUIRE(updatedObj.is_selected == true);
    }

    SECTION("Load models from database") {
        ModelData loadModel {5, "LoadModel", "./load/path", "{}", "Load Title", {}, "Load Author", "/load/file/path", "LoadLibrary", false, false};
        REQUIRE(model.insertModel(loadModel.id, loadModel) == true);

        model.refreshModelData();
        REQUIRE(model.modelExists(loadModel.id) == true);
    }

    // Clean up after tests
    cleanupTestDirectory(testDir);
}

TEST_CASE("Extended Model Edge Cases", "[Model]") {
    std::string testDir = setupTestDirectory();
    cleanupTestDirectory(testDir);
    std::filesystem::create_directories(testDir);
    Model model(testDir);

    SECTION("Attempt to Insert Duplicate Model IDs") {
        ModelData modelData {6, "DuplicateModel", "./duplicate/path", "{}", "Duplicate Title", {}, "Author", "/duplicate/path", "Library", false, false};
        REQUIRE(model.insertModel(modelData.id, modelData) == true);

        // Try inserting again with the same ID
        REQUIRE_FALSE(model.insertModel(modelData.id, modelData));
    }

    SECTION("Insert and Retrieve Nested Objects") {
        ModelData nestedModel {7, "NestedModel", "./nested/path", "{}", "Nested Title", {}, "Nested Author", "/nested/path", "NestedLibrary", false, false};
        REQUIRE(model.insertModel(nestedModel.id, nestedModel) == true);

        ObjectData parentObject {0, nestedModel.id, "ParentObject", -1, false};
        int parent_id = model.insertObject(parentObject);
        REQUIRE(parent_id != -1);

        ObjectData childObject {0, nestedModel.id, "ChildObject", parent_id, false};
        int child_id = model.insertObject(childObject);
        REQUIRE(child_id != -1);

        auto objects = model.getObjectsForModel(nestedModel.id);
        REQUIRE(objects.size() == 2);
        REQUIRE(objects[0].name == "ParentObject");
        REQUIRE(objects[1].name == "ChildObject");
        REQUIRE(objects[1].parent_object_id == parent_id);
    }

    SECTION("Validate Model Data on Refresh") {
        ModelData refreshModel {8, "RefreshModel", "./refresh/path", "{}", "Refresh Title", {}, "Author", "/refresh/path", "Library", true, false};
        REQUIRE(model.insertModel(refreshModel.id, refreshModel) == true);

        // Refresh the data and check consistency
        model.refreshModelData();
        auto fetchedModel = model.getModelById(refreshModel.id);
        REQUIRE(fetchedModel.short_name == refreshModel.short_name);
        REQUIRE(fetchedModel.is_selected == true);
    }

    SECTION("Handle SQL Injection in Model Insertion") {
        ModelData injectionModel {9, "InjectionModel", "./injection/path; DROP TABLE models;", "{}", "Injection Title", {}, "Author", "/injection/path", "Library", false, false};
        REQUIRE(model.insertModel(injectionModel.id, injectionModel) == true);

        auto fetchedModel = model.getModelById(injectionModel.id);
        REQUIRE(fetchedModel.short_name == "InjectionModel");  // Ensures no harm from SQL injection attempt
    }

    cleanupTestDirectory(testDir);
}

TEST_CASE("Model Data Roles", "[Model]") {
    std::string testDir = setupTestDirectory();
    cleanupTestDirectory(testDir);
    std::filesystem::create_directories(testDir);
    Model model(testDir);

    ModelData testModel {1, "TestModel", "./path/to/primary/file", "{\"info\": true}",
                        "Test Title", {}, "Author", "/file/path", "Library", true, false};
    testModel.thumbnail = std::vector<char>({'\x89', 'P', 'N', 'G'}); // Simulated PNG header
    REQUIRE(model.insertModel(testModel.id, testModel) == true);

    QModelIndex index = model.index(0, 0);
    REQUIRE(index.isValid());

    SECTION("Display Role - Short Name") {
        QVariant shortName = model.data(index, Qt::DisplayRole);
        REQUIRE(shortName.toString().toStdString() == "TestModel");
    }

    SECTION("Id Role") {
        QVariant id = model.data(index, Model::IdRole);
        REQUIRE(id.toInt() == testModel.id);
    }

    SECTION("Primary File Role") {
        QVariant primaryFile = model.data(index, Model::PrimaryFileRole);
        REQUIRE(primaryFile.toString().toStdString() == testModel.primary_file);
    }

    SECTION("Override Info Role") {
        QVariant overrideInfo = model.data(index, Model::OverrideInfoRole);
        REQUIRE(overrideInfo.toString().toStdString() == testModel.override_info);
    }

    SECTION("Title Role") {
        QVariant title = model.data(index, Model::TitleRole);
        REQUIRE(title.toString().toStdString() == testModel.title);
    }

    // SECTION("Thumbnail Role") {
    //     QVariant thumbnail = model.data(index, Model::ThumbnailRole);
    //     REQUIRE(!thumbnail.isNull());  // Ensure thumbnail data is not null
    // }

    SECTION("Author Role") {
        QVariant author = model.data(index, Model::AuthorRole);
        REQUIRE(author.toString().toStdString() == testModel.author);
    }

    SECTION("File Path Role") {
        QVariant filePath = model.data(index, Model::FilePathRole);
        REQUIRE(filePath.toString().toStdString() == testModel.file_path);
    }

    SECTION("Library Name Role") {
        QVariant libraryName = model.data(index, Model::LibraryNameRole);
        REQUIRE(libraryName.toString().toStdString() == testModel.library_name);
    }

    SECTION("Is Selected Role") {
        QVariant isSelected = model.data(index, Model::IsSelectedRole);
        REQUIRE(isSelected.toBool() == testModel.is_selected);
    }

    cleanupTestDirectory(testDir);
}

TEST_CASE("Role Names Mapping", "[Model]") {
    Model model("");
    auto roles = model.roleNames();

    SECTION("IdRole Mapping") {
        REQUIRE(roles[Model::IdRole] == "id");
    }

    SECTION("ShortNameRole Mapping") {
        REQUIRE(roles[Model::ShortNameRole] == "short_name");
    }

    SECTION("PrimaryFileRole Mapping") {
        REQUIRE(roles[Model::PrimaryFileRole] == "primary_file");
    }

    SECTION("OverrideInfoRole Mapping") {
        REQUIRE(roles[Model::OverrideInfoRole] == "override_info");
    }

    SECTION("TitleRole Mapping") {
        REQUIRE(roles[Model::TitleRole] == "title");
    }

    SECTION("ThumbnailRole Mapping") {
        REQUIRE(roles[Model::ThumbnailRole] == "thumbnail");
    }

    SECTION("AuthorRole Mapping") {
        REQUIRE(roles[Model::AuthorRole] == "author");
    }

    SECTION("FilePathRole Mapping") {
        REQUIRE(roles[Model::FilePathRole] == "file_path");
    }

    SECTION("LibraryNameRole Mapping") {
        REQUIRE(roles[Model::LibraryNameRole] == "library_name");
    }

    SECTION("IsSelectedRole Mapping") {
        REQUIRE(roles[Model::IsSelectedRole] == "isSelected");
    }
}
