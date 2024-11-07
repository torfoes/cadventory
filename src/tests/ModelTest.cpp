#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include "Model.h"
#include <filesystem>
#include <iostream>
#include <memory>

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

TEST_CASE("Model Hashing Functionality", "[Model]") {
    std::string testDir = setupTestDirectory();
    cleanupTestDirectory(testDir); 
    std::filesystem::create_directories(testDir);

    Model model(testDir);
    std::string tempFilePath = testDir + "/testfile.txt";

    SECTION("Hash of an existing file") {
        // Create a temporary file
        std::ofstream outFile(tempFilePath);
        outFile << "Test content for hashing";
        outFile.close();

        // Call hashModel
        int hashValue = model.hashModel(tempFilePath);
        REQUIRE(hashValue != 0);  // Ensure a valid hash is returned
    }

    SECTION("Hash of a non-existent file") {
        // Call hashModel on a non-existent file
        int hashValue = model.hashModel(testDir + "/nonexistent.txt");
        REQUIRE(hashValue == 0);  // Hash should be zero as file doesn't exist
    }

    cleanupTestDirectory(testDir);  // Clean up after test
}

TEST_CASE("Model setData Function", "[Model]") {
    std::string testDir = setupTestDirectory();
    cleanupTestDirectory(testDir);
    std::filesystem::create_directories(testDir);

    Model model(testDir);

    // Insert a sample model
    ModelData sampleModel {1, "SampleModel", "./path/to/model", "{}", "Sample Title", {}, "Author", "/path", "Library", true, false};
    REQUIRE(model.insertModel(sampleModel.id, sampleModel) == true);

    // Set up a valid index and update `is_selected` role
    QModelIndex index = model.index(0, 0);
    REQUIRE(index.isValid());

    REQUIRE(model.setData(index, QVariant(false), Model::IsSelectedRole) == true);
    auto updatedModel = model.getModelById(sampleModel.id);
    REQUIRE(updatedModel.is_selected == false);

    // Test an invalid index
    QModelIndex invalidIndex;
    REQUIRE(model.setData(invalidIndex, QVariant(true), Model::IsSelectedRole) == false);

    cleanupTestDirectory(testDir);
}

TEST_CASE("Get Selected Models from Model", "[Model]") {
    std::string testDir = setupTestDirectory();
    cleanupTestDirectory(testDir);
    std::filesystem::create_directories(testDir);

    Model model(testDir);

    // Manually insert test data to `models` list with varied selection status
    ModelData model1 = {1, "Model1", "./path/to/model1", "{}", "Title1", {}, "Author1", "/path1", "Library1", true, false};
    ModelData model2 = {2, "Model2", "./path/to/model2", "{}", "Title2", {}, "Author2", "/path2", "Library2", false, false};
    ModelData model3 = {3, "Model3", "./path/to/model3", "{}", "Title3", {}, "Author3", "/path3", "Library3", true, false};
    ModelData model4 = {4, "Model4", "./path/to/model4", "{}", "Title4", {}, "Author4", "/path4", "Library4", false, false};

    // Insert each model directly using the known `insertModel` function
    REQUIRE(model.insertModel(model1.id, model1) == true);
    REQUIRE(model.insertModel(model2.id, model2) == true);
    REQUIRE(model.insertModel(model3.id, model3) == true);
    REQUIRE(model.insertModel(model4.id, model4) == true);

    // Test case: Retrieve only selected models
    auto selectedModels = model.getSelectedModels();
    REQUIRE(selectedModels.size() == 2);  // Expect only model1 and model3 to be selected

    // Verify each selected model is correct
    std::vector<int> expectedIds = {1, 3};
    for (const auto& selectedModel : selectedModels) {
        REQUIRE(std::find(expectedIds.begin(), expectedIds.end(), selectedModel.id) != expectedIds.end());
        REQUIRE(selectedModel.is_selected == true);
    }

    cleanupTestDirectory(testDir);
}

TEST_CASE("Get Selected Objects for Model", "[Model]") {
    std::string testDir = setupTestDirectory();
    cleanupTestDirectory(testDir);
    std::filesystem::create_directories(testDir);

    Model model(testDir);

    // Insert a model for testing
    ModelData testModel = {1, "TestModel", "./path/to/model", "{}", "Test Title", {}, "Author", "/file/path", "Library", true, false};
    REQUIRE(model.insertModel(testModel.id, testModel) == true);

    // Insert ObjectData associated with the model, with varied selection states
    ObjectData object1 = {1, testModel.id, "Object1", -1, true};   // Selected
    ObjectData object2 = {2, testModel.id, "Object2", -1, false};  // Not selected
    ObjectData object3 = {3, testModel.id, "Object3", -1, true};   // Selected
    ObjectData object4 = {4, testModel.id, "Object4", -1, false};  // Not selected

    REQUIRE(model.insertObject(object1));  // Insert using verified existing insert function
    REQUIRE(model.insertObject(object2));
    REQUIRE(model.insertObject(object3));
    REQUIRE(model.insertObject(object4));

    SECTION("Retrieve only selected objects for a given model ID") {
        auto selectedObjects = model.getSelectedObjectsForModel(testModel.id);
        REQUIRE(selectedObjects.size() == 2);  // Only object1 and object3 should be selected

        std::vector<int> expectedIds = {1, 3};
        for (const auto& selectedObject : selectedObjects) {
            REQUIRE(std::find(expectedIds.begin(), expectedIds.end(), selectedObject.object_id) != expectedIds.end());
            REQUIRE(selectedObject.is_selected == true);
        }
    }

    SECTION("No objects selected for the given model ID") {
        // Deselect all objects in the test model
        object1.is_selected = false;
        object3.is_selected = false;
        REQUIRE(model.updateObject(object1));
        REQUIRE(model.updateObject(object3));

        auto selectedObjects = model.getSelectedObjectsForModel(testModel.id);
        REQUIRE(selectedObjects.empty());  // No selected objects should be returned
    }

    SECTION("All objects selected for the given model ID") {
        // Select all objects in the test model
        object1.is_selected = true;
        object2.is_selected = true;
        object3.is_selected = true;
        object4.is_selected = true;

        REQUIRE(model.updateObject(object1));
        REQUIRE(model.updateObject(object2));
        REQUIRE(model.updateObject(object3));
        REQUIRE(model.updateObject(object4));

        auto selectedObjects = model.getSelectedObjectsForModel(testModel.id);
        REQUIRE(selectedObjects.size() == 4);  // All objects should be selected

        std::vector<int> expectedIds = {1, 2, 3, 4};
        for (const auto& selectedObject : selectedObjects) {
            REQUIRE(std::find(expectedIds.begin(), expectedIds.end(), selectedObject.object_id) != expectedIds.end());
            REQUIRE(selectedObject.is_selected == true);
        }
    }

    cleanupTestDirectory(testDir);
}

TEST_CASE("Model Transaction Management", "[Model]") {
    std::string testDir = setupTestDirectory();
    cleanupTestDirectory(testDir);
    std::filesystem::create_directories(testDir);

    Model model(testDir);

    // Insert a test model
    ModelData testModel = {1, "TestModel", "./path/to/model", "{}", "Test Title", {}, "Author", "/file/path", "Library", true, false};
    REQUIRE(model.insertModel(testModel.id, testModel) == true);

    // Insert an object associated with the model
    ObjectData object = {1, testModel.id, "Object", -1, false};
    REQUIRE(model.insertObject(object) == true);

    SECTION("Begin and Commit Transaction") {
        // Test beginning a transaction (should not throw or fail)
        REQUIRE_NOTHROW(model.beginTransaction());
        
        // Simulate a change within the transaction
        object.is_selected = true;
        REQUIRE(model.updateObject(object) == true);

        // Commit the transaction
        REQUIRE_NOTHROW(model.commitTransaction());
        
        // Verify the change persisted
        auto selectedObjects = model.getSelectedObjectsForModel(testModel.id);
        REQUIRE(selectedObjects.size() == 1);
        REQUIRE(selectedObjects[0].object_id == object.object_id);
    }

    cleanupTestDirectory(testDir);
}

TEST_CASE("Update Object Parent ID", "[Model]") {
    std::string testDir = setupTestDirectory();
    cleanupTestDirectory(testDir);
    std::filesystem::create_directories(testDir);

    Model model(testDir);

    // Insert a test model
    ModelData testModel = {1, "TestModel", "./path/to/model", "{}", "Test Title", {}, "Author", "/file/path", "Library", true, false};
    REQUIRE(model.insertModel(testModel.id, testModel) == true);

    // Insert objects associated with the model and set is_selected to true
    ObjectData object1 = {1, testModel.id, "Object1", -1, true};
    ObjectData object2 = {2, testModel.id, "Object2", -1, true}; // Set is_selected = true to match retrieval criteria

    bool insertResult1 = model.insertObject(object1);
    bool insertResult2 = model.insertObject(object2);

    REQUIRE(insertResult1 == true);  
    REQUIRE(insertResult2 == true); 

    // Confirm objects are retrievable with `is_selected` as true
    auto objectsBeforeUpdate = model.getSelectedObjectsForModel(testModel.id);
    REQUIRE(std::find_if(objectsBeforeUpdate.begin(), objectsBeforeUpdate.end(), [&](const ObjectData& obj) {
        return obj.object_id == object2.object_id;
    }) != objectsBeforeUpdate.end());

    SECTION("Update parent ID successfully") {
        int newParentId = object1.object_id;

        // Start a transaction, update the parent ID, and commit
        model.beginTransaction();
        REQUIRE(model.updateObjectParentId(object2.object_id, newParentId) == true);
        model.commitTransaction();

        // Fetch updated object and verify parent ID change
        auto selectedObjects = model.getSelectedObjectsForModel(testModel.id);
        auto updatedObject = std::find_if(selectedObjects.begin(), selectedObjects.end(), [&](const ObjectData& obj) {
            return obj.object_id == object2.object_id;
        });

        REQUIRE(updatedObject != selectedObjects.end());
        REQUIRE(updatedObject->parent_object_id == newParentId);
    }

    SECTION("Attempt to update parent ID with invalid object ID") {
        int invalidObjectId = 9999; // Assumed non-existent
        int newParentId = object1.object_id;

        model.beginTransaction();
        bool updateResult = model.updateObjectParentId(invalidObjectId, newParentId);
        model.commitTransaction();

        REQUIRE(updateResult == true);

        // Ensure that no existing object's parent ID has been incorrectly set to newParentId
        auto allObjects = model.getSelectedObjectsForModel(testModel.id);
        for (const auto& obj : allObjects) {
            REQUIRE(obj.object_id != invalidObjectId); // invalid object should not exist
            REQUIRE(obj.parent_object_id != newParentId); // ensure parent ID of valid objects is unchanged
        }
    }

    cleanupTestDirectory(testDir);
}