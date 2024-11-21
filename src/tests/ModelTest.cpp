// #define CATCH_CONFIG_MAIN
// #include <catch2/catch_test_macros.hpp>
// #include <fstream>
// #include "Model.h"
// #include <filesystem>
// #include <iostream>
// #include <memory>

// // Helper function for testing directory
// std::string setupTestDirectory() {
//     auto tempDir = std::filesystem::temp_directory_path() / "cadventory_test";
//     if (!std::filesystem::exists(tempDir)) {
//         std::filesystem::create_directories(tempDir);
//     }
//     return tempDir.string();
// }

// // Cleanup function for the test directory
// void cleanupTestDirectory(const std::string& path) {
//     if (std::filesystem::exists(path)) {
//         std::filesystem::remove_all(path);
//     }
// }

// TEST_CASE("Model Operations", "[Model]") {
//     std::string testDir = setupTestDirectory();
//     cleanupTestDirectory(testDir);  // Clean before starting

//     // Re-create the directory for test purposes
//     std::filesystem::create_directories(testDir);

//     // Now initialize the Model with the test directory
//     Model model(testDir);

//     SECTION("Database Initialization") {
//         // Ensure the hidden directory and database file exist
//         REQUIRE(std::filesystem::exists(model.getHiddenDirectoryPath()));
//         REQUIRE(std::filesystem::exists(testDir + "/.cadventory/metadata.db"));
//     }

//     SECTION("Insert and verify model") {
//         ModelData newModel {"TestModel", "./path/to/cad/file", "{}", "Test Title", {}, "Author", "/file/path", "Library", true, false, false};
//         REQUIRE(model.insertModel(newModel) == true);

//         // Fetch the inserted model by file_path since IDs are auto-incremented
//         auto fetchedModel = model.getModelByFilePath(newModel.file_path);
//         REQUIRE(fetchedModel.short_name == "TestModel");
//         REQUIRE(fetchedModel.file_path == "/file/path");
//         REQUIRE(fetchedModel.library_name == "Library");
//         REQUIRE(fetchedModel.is_selected == true);
//     }

//     SECTION("Update model attributes") {
//         ModelData newModel {"UpdatedModel", "./new/path", "{\"updated\":true}", "Updated Title", {}, "New Author", "/new/file/path", "NewLibrary", false, false, false};
//         REQUIRE(model.insertModel(newModel) == true);

//         // Fetch the inserted model
//         auto insertedModel = model.getModelByFilePath(newModel.file_path);
//         int modelId = insertedModel.id;

//         // Update the model
//         insertedModel.short_name = "ModifiedModel";
//         insertedModel.library_name = "ModifiedLibrary";
//         REQUIRE(model.updateModel(modelId, insertedModel) == true);

//         auto fetchedModel = model.getModelById(modelId);
//         REQUIRE(fetchedModel.short_name == "ModifiedModel");
//         REQUIRE(fetchedModel.library_name == "ModifiedLibrary");
//         REQUIRE(fetchedModel.is_selected == false);
//     }

//     SECTION("Delete model") {
//         ModelData deleteModel {"DeleteModel", "./delete/path", "{}", "Delete Title", {}, "Delete Author", "/delete/file/path", "DeleteLibrary", false, false, false};
//         REQUIRE(model.insertModel(deleteModel) == true);

//         // Fetch the inserted model
//         auto insertedModel = model.getModelByFilePath(deleteModel.file_path);
//         int modelId = insertedModel.id;

//         REQUIRE(model.modelExists(modelId) == true);

//         REQUIRE(model.deleteModel(modelId) == true);
//         REQUIRE(model.modelExists(modelId) == false);
//     }

//     SECTION("Add and retrieve objects for model") {
//         ModelData objModel {"ObjModel", "./obj/path", "{}", "Obj Title", {}, "Obj Author", "/obj/file/path", "ObjLibrary", false, false, false};
//         REQUIRE(model.insertModel(objModel) == true);

//         // Fetch the inserted model
//         auto insertedModel = model.getModelByFilePath(objModel.file_path);
//         int modelId = insertedModel.id;

//         ObjectData obj1 {0, modelId, "Object1", -1, false};
//         int obj1_id = model.insertObject(obj1);
//         REQUIRE(obj1_id != -1);

//         ObjectData obj2 {0, modelId, "Object2", obj1_id, true};
//         int obj2_id = model.insertObject(obj2);
//         REQUIRE(obj2_id != -1);

//         auto objects = model.getObjectsForModel(modelId);
//         REQUIRE(objects.size() == 2);
//         REQUIRE(objects[0].name == "Object1");
//         REQUIRE(objects[1].name == "Object2");
//         REQUIRE(objects[1].parent_object_id == obj1_id);
//         REQUIRE(objects[1].is_selected == true);
//     }

//     SECTION("Update and select objects") {
//         ModelData objModel {"UpdateObjModel", "./update/path", "{}", "Update Obj Title", {}, "Update Obj Author", "/update/file/path", "UpdateLibrary", false, false, false};
//         REQUIRE(model.insertModel(objModel) == true);

//         // Fetch the inserted model
//         auto insertedModel = model.getModelByFilePath(objModel.file_path);
//         int modelId = insertedModel.id;

//         ObjectData obj {0, modelId, "SelectableObject", -1, false};
//         int obj_id = model.insertObject(obj);
//         REQUIRE(obj_id != -1);

//         REQUIRE(model.updateObjectSelection(obj_id, true) == true);
//         auto updatedObj = model.getObjectById(obj_id);
//         REQUIRE(updatedObj.is_selected == true);
//     }

//     SECTION("Load models from database") {
//         ModelData loadModel {"LoadModel", "./load/path", "{}", "Load Title", {}, "Load Author", "/load/file/path", "LoadLibrary", false, false, false};
//         REQUIRE(model.insertModel(loadModel) == true);

//         model.refreshModelData();

//         // Fetch the model by file_path
//         auto fetchedModel = model.getModelByFilePath(loadModel.file_path);
//         REQUIRE(fetchedModel.short_name == "LoadModel");
//     }

//     // Clean up after tests
//     cleanupTestDirectory(testDir);
// }

// TEST_CASE("Extended Model Edge Cases", "[Model]") {
//     std::string testDir = setupTestDirectory();
//     cleanupTestDirectory(testDir);
//     std::filesystem::create_directories(testDir);
//     Model model(testDir);

//     SECTION("Attempt to Insert Duplicate short_name") {
//         ModelData modelData1 {"DuplicateModel", "./duplicate/path1", "{}", "Duplicate Title 1", {}, "Author", "/duplicate/path1", "Library", false, false, false};
//         REQUIRE(model.insertModel(modelData1) == true);

//         ModelData modelData2 {"DuplicateModel", "./duplicate/path2", "{}", "Duplicate Title 2", {}, "Author", "/duplicate/path2", "Library", false, false, false};
//         REQUIRE(model.insertModel(modelData2) == true);

//         // Fetch both models
//         auto fetchedModel1 = model.getModelByFilePath(modelData1.file_path);
//         auto fetchedModel2 = model.getModelByFilePath(modelData2.file_path);

//         REQUIRE(fetchedModel1.short_name == "DuplicateModel");
//         REQUIRE(fetchedModel2.short_name == "DuplicateModel_1"); // Should have been enumerated
//     }

//     SECTION("Attempt to Insert Duplicate file_path") {
//         ModelData modelData {"UniqueModel", "./unique/path", "{}", "Unique Title", {}, "Author", "/unique/path", "Library", false, false, false};
//         REQUIRE(model.insertModel(modelData) == true);

//         // Attempt to insert another model with the same file_path
//         ModelData duplicateModelData {"AnotherModel", "./another/path", "{}", "Another Title", {}, "Author", "/unique/path", "Library", false, false, false};
//         REQUIRE_FALSE(model.insertModel(duplicateModelData)); // Should fail due to unique file_path constraint
//     }

//     SECTION("Insert and Retrieve Nested Objects") {
//         ModelData nestedModel {"NestedModel", "./nested/path", "{}", "Nested Title", {}, "Nested Author", "/nested/path", "NestedLibrary", false, false, false};
//         REQUIRE(model.insertModel(nestedModel) == true);

//         auto insertedModel = model.getModelByFilePath(nestedModel.file_path);
//         int modelId = insertedModel.id;

//         ObjectData parentObject {0, modelId, "ParentObject", -1, false};
//         int parent_id = model.insertObject(parentObject);
//         REQUIRE(parent_id != -1);

//         ObjectData childObject {0, modelId, "ChildObject", parent_id, false};
//         int child_id = model.insertObject(childObject);
//         REQUIRE(child_id != -1);

//         auto objects = model.getObjectsForModel(modelId);
//         REQUIRE(objects.size() == 2);
//         REQUIRE(objects[0].name == "ParentObject");
//         REQUIRE(objects[1].name == "ChildObject");
//         REQUIRE(objects[1].parent_object_id == parent_id);
//     }

//     SECTION("Validate Model Data on Refresh") {
//         ModelData refreshModel {"RefreshModel", "./refresh/path", "{}", "Refresh Title", {}, "Author", "/refresh/path", "Library", true, false, false};
//         REQUIRE(model.insertModel(refreshModel) == true);

//         // Refresh the data and check consistency
//         model.refreshModelData();
//         auto fetchedModel = model.getModelByFilePath(refreshModel.file_path);
//         REQUIRE(fetchedModel.short_name == refreshModel.short_name);
//         REQUIRE(fetchedModel.is_selected == true);
//     }

//     SECTION("Handle SQL Injection in Model Insertion") {
//         ModelData injectionModel {"InjectionModel", "./injection/path; DROP TABLE models;", "{}", "Injection Title", {}, "Author", "/injection/path", "Library", false, false, false};
//         REQUIRE(model.insertModel(injectionModel) == true);

//         auto fetchedModel = model.getModelByFilePath(injectionModel.file_path);
//         REQUIRE(fetchedModel.short_name == "InjectionModel");  // Ensures no harm from SQL injection attempt
//     }

//     cleanupTestDirectory(testDir);
// }

// TEST_CASE("Model Data Roles", "[Model]") {
//     std::string testDir = setupTestDirectory();
//     cleanupTestDirectory(testDir);
//     std::filesystem::create_directories(testDir);
//     Model model(testDir);

//     ModelData testModel {"TestModel", "./path/to/primary/file", "{\"info\": true}",
//                         "Test Title", {}, "Author", "/file/path", "Library", true, false, false};
//     testModel.thumbnail = std::vector<char>({'\x89', 'P', 'N', 'G'}); // Simulated PNG header
//     REQUIRE(model.insertModel(testModel) == true);

//     QModelIndex index = model.index(0, 0);
//     REQUIRE(index.isValid());

//     SECTION("Display Role - Short Name") {
//         QVariant shortName = model.data(index, Qt::DisplayRole);
//         REQUIRE(shortName.toString().toStdString() == "TestModel");
//     }

//     SECTION("Id Role") {
//         QVariant id = model.data(index, Model::IdRole);
//         REQUIRE(id.toInt() > 0); // ID should be auto-incremented and greater than 0
//     }

//     SECTION("Primary File Role") {
//         QVariant primaryFile = model.data(index, Model::PrimaryFileRole);
//         REQUIRE(primaryFile.toString().toStdString() == testModel.primary_file);
//     }

//     SECTION("Override Info Role") {
//         QVariant overrideInfo = model.data(index, Model::OverrideInfoRole);
//         REQUIRE(overrideInfo.toString().toStdString() == testModel.override_info);
//     }

//     SECTION("Title Role") {
//         QVariant title = model.data(index, Model::TitleRole);
//         REQUIRE(title.toString().toStdString() == testModel.title);
//     }

//     // SECTION("Thumbnail Role") {
//     //     QVariant thumbnail = model.data(index, Model::ThumbnailRole);
//     //     REQUIRE(!thumbnail.isNull());  // Ensure thumbnail data is not null
//     // }

//     SECTION("Author Role") {
//         QVariant author = model.data(index, Model::AuthorRole);
//         REQUIRE(author.toString().toStdString() == testModel.author);
//     }

//     SECTION("File Path Role") {
//         QVariant filePath = model.data(index, Model::FilePathRole);
//         REQUIRE(filePath.toString().toStdString() == testModel.file_path);
//     }

//     SECTION("Library Name Role") {
//         QVariant libraryName = model.data(index, Model::LibraryNameRole);
//         REQUIRE(libraryName.toString().toStdString() == testModel.library_name);
//     }

//     SECTION("Is Selected Role") {
//         QVariant isSelected = model.data(index, Model::IsSelectedRole);
//         REQUIRE(isSelected.toBool() == testModel.is_selected);
//     }

//     cleanupTestDirectory(testDir);
// }

// TEST_CASE("Role Names Mapping", "[Model]") {
//     Model model("");
//     auto roles = model.roleNames();

//     SECTION("IdRole Mapping") {
//         REQUIRE(roles[Model::IdRole] == "id");
//     }

//     SECTION("ShortNameRole Mapping") {
//         REQUIRE(roles[Model::ShortNameRole] == "short_name");
//     }

//     SECTION("PrimaryFileRole Mapping") {
//         REQUIRE(roles[Model::PrimaryFileRole] == "primary_file");
//     }

//     SECTION("OverrideInfoRole Mapping") {
//         REQUIRE(roles[Model::OverrideInfoRole] == "override_info");
//     }

//     SECTION("TitleRole Mapping") {
//         REQUIRE(roles[Model::TitleRole] == "title");
//     }

//     SECTION("ThumbnailRole Mapping") {
//         REQUIRE(roles[Model::ThumbnailRole] == "thumbnail");
//     }

//     SECTION("AuthorRole Mapping") {
//         REQUIRE(roles[Model::AuthorRole] == "author");
//     }

//     SECTION("FilePathRole Mapping") {
//         REQUIRE(roles[Model::FilePathRole] == "file_path");
//     }

//     SECTION("LibraryNameRole Mapping") {
//         REQUIRE(roles[Model::LibraryNameRole] == "library_name");
//     }

//     SECTION("IsSelectedRole Mapping") {
//         REQUIRE(roles[Model::IsSelectedRole] == "is_selected");
//     }
// }

// TEST_CASE("Model Hashing Functionality", "[Model]") {
//     std::string testDir = setupTestDirectory();
//     cleanupTestDirectory(testDir);
//     std::filesystem::create_directories(testDir);

//     Model model(testDir);
//     std::string tempFilePath = testDir + "/testfile.txt";

//     SECTION("Hash of an existing file") {
//         // Create a temporary file
//         std::ofstream outFile(tempFilePath);
//         outFile << "Test content for hashing";
//         outFile.close();

//         // Call hashModel
//         int hashValue = model.hashModel(tempFilePath);
//         REQUIRE(hashValue != 0);  // Ensure a valid hash is returned
//     }

//     SECTION("Hash of a non-existent file") {
//         // Call hashModel on a non-existent file
//         int hashValue = model.hashModel(testDir + "/nonexistent.txt");
//         REQUIRE(hashValue == 0);  // Hash should be zero as file doesn't exist
//     }

//     cleanupTestDirectory(testDir);  // Clean up after test
// }

// TEST_CASE("Model setData Function", "[Model]") {
//     std::string testDir = setupTestDirectory();
//     cleanupTestDirectory(testDir);
//     std::filesystem::create_directories(testDir);

//     Model model(testDir);

//     // Insert a sample model
//     ModelData sampleModel {"SampleModel", "./path/to/model", "{}", "Sample Title", {}, "Author", "/path", "Library", true, false, false};
//     REQUIRE(model.insertModel(sampleModel) == true);

//     QModelIndex index = model.index(0, 0);
//     REQUIRE(index.isValid());

//     REQUIRE(model.setData(index, QVariant(false), Model::IsSelectedRole) == true);
//     auto updatedModel = model.getModelByFilePath(sampleModel.file_path);
//     REQUIRE(updatedModel.is_selected == false);

//     // Test an invalid index
//     QModelIndex invalidIndex;
//     REQUIRE(model.setData(invalidIndex, QVariant(true), Model::IsSelectedRole) == false);

//     cleanupTestDirectory(testDir);
// }

// TEST_CASE("Get Selected Models from Model", "[Model]") {
//     std::string testDir = setupTestDirectory();
//     cleanupTestDirectory(testDir);
//     std::filesystem::create_directories(testDir);

//     Model model(testDir);

//     // Insert models with varied selection status
//     ModelData model1 {"Model1", "./path/to/model1", "{}", "Title1", {}, "Author1", "/path1", "Library1", true, false, false};
//     ModelData model2 {"Model2", "./path/to/model2", "{}", "Title2", {}, "Author2", "/path2", "Library2", false, false, false};
//     ModelData model3 {"Model3", "./path/to/model3", "{}", "Title3", {}, "Author3", "/path3", "Library3", true, false, false};
//     ModelData model4 {"Model4", "./path/to/model4", "{}", "Title4", {}, "Author4", "/path4", "Library4", false, false, false};

//     REQUIRE(model.insertModel(model1) == true);
//     REQUIRE(model.insertModel(model2) == true);
//     REQUIRE(model.insertModel(model3) == true);
//     REQUIRE(model.insertModel(model4) == true);

//     // Test case: Retrieve only selected models
//     auto selectedModels = model.getSelectedModels();
//     REQUIRE(selectedModels.size() == 2);  // Expect only model1 and model3 to be selected

//     // Verify each selected model is correct
//     std::vector<std::string> expectedNames = {"Model1", "Model3"};
//     for (const auto& selectedModel : selectedModels) {
//         REQUIRE(std::find(expectedNames.begin(), expectedNames.end(), selectedModel.short_name) != expectedNames.end());
//         REQUIRE(selectedModel.is_selected == true);
//     }

//     cleanupTestDirectory(testDir);
// }

// TEST_CASE("Get Selected Objects for Model", "[Model]") {
//     std::string testDir = setupTestDirectory();
//     cleanupTestDirectory(testDir);
//     std::filesystem::create_directories(testDir);

//     Model model(testDir);

//     // Insert a model for testing
//     ModelData testModel {"TestModel", "./path/to/model", "{}", "Test Title", {}, "Author", "/file/path", "Library", true, false, false};
//     REQUIRE(model.insertModel(testModel) == true);

//     // Fetch the model to get its ID
//     auto insertedModel = model.getModelByFilePath(testModel.file_path);
//     int modelId = insertedModel.id;

//     // Insert ObjectData associated with the model, with varied selection states
//     ObjectData object1 {0, modelId, "Object1", -1, true};   // Selected
//     ObjectData object2 {0, modelId, "Object2", -1, false};  // Not selected
//     ObjectData object3 {0, modelId, "Object3", -1, true};   // Selected
//     ObjectData object4 {0, modelId, "Object4", -1, false};  // Not selected

//     int obj1_id = model.insertObject(object1);
//     int obj2_id = model.insertObject(object2);
//     int obj3_id = model.insertObject(object3);
//     int obj4_id = model.insertObject(object4);

//     SECTION("Retrieve only selected objects for a given model ID") {
//         auto selectedObjects = model.getSelectedObjectsForModel(modelId);
//         REQUIRE(selectedObjects.size() == 2);  // Only object1 and object3 should be selected

//         std::vector<std::string> expectedNames = {"Object1", "Object3"};
//         for (const auto& selectedObject : selectedObjects) {
//             REQUIRE(std::find(expectedNames.begin(), expectedNames.end(), selectedObject.name) != expectedNames.end());
//             REQUIRE(selectedObject.is_selected == true);
//         }
//     }

//     cleanupTestDirectory(testDir);
// }

// TEST_CASE("Model Transaction Management", "[Model]") {
//     std::string testDir = setupTestDirectory();
//     cleanupTestDirectory(testDir);
//     std::filesystem::create_directories(testDir);

//     Model model(testDir);

//     // Insert a test model
//     ModelData testModel {"TestModel", "./path/to/model", "{}", "Test Title", {}, "Author", "/file/path", "Library", true, false, false};
//     REQUIRE(model.insertModel(testModel) == true);

//     auto insertedModel = model.getModelByFilePath(testModel.file_path);
//     int modelId = insertedModel.id;

//     // Insert an object associated with the model
//     ObjectData object {0, modelId, "Object", -1, false};
//     int obj_id = model.insertObject(object);
//     REQUIRE(obj_id != -1);

//     SECTION("Begin and Commit Transaction") {
//         // Start a transaction, update the object, and commit
//         model.beginTransaction();
//         REQUIRE(model.updateObjectSelection(obj_id, true) == true);
//         model.commitTransaction();

//         // Verify the change persisted
//         auto selectedObjects = model.getSelectedObjectsForModel(modelId);
//         REQUIRE(selectedObjects.size() == 1);
//         REQUIRE(selectedObjects[0].object_id == obj_id);
//     }

//     cleanupTestDirectory(testDir);
// }

// TEST_CASE("Update Object Parent ID", "[Model]") {
//     std::string testDir = setupTestDirectory();
//     cleanupTestDirectory(testDir);
//     std::filesystem::create_directories(testDir);

//     Model model(testDir);

//     // Insert a test model
//     ModelData testModel {"TestModel", "./path/to/model", "{}", "Test Title", {}, "Author", "/file/path", "Library", true, false, false};
//     REQUIRE(model.insertModel(testModel) == true);

//     auto insertedModel = model.getModelByFilePath(testModel.file_path);
//     int modelId = insertedModel.id;

//     // Insert objects associated with the model
//     ObjectData object1 {0, modelId, "Object1", -1, true};
//     ObjectData object2 {0, modelId, "Object2", -1, true};

//     int obj1_id = model.insertObject(object1);
//     int obj2_id = model.insertObject(object2);

//     SECTION("Update parent ID successfully") {
//         // Update the parent ID of object2 to be object1
//         model.beginTransaction();
//         REQUIRE(model.updateObjectParentId(obj2_id, obj1_id) == true);
//         model.commitTransaction();

//         // Fetch updated object and verify parent ID change
//         auto updatedObject = model.getObjectById(obj2_id);
//         REQUIRE(updatedObject.parent_object_id == obj1_id);
//     }

//     cleanupTestDirectory(testDir);
// }
