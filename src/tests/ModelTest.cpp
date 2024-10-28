
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

// Setup helper function
void setupTestDB(const std::string& path) {
    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
    }
}

TEST_CASE("Model Operations", "[Model]") {
    std::string testDB = "test_models.db";
    setupTestDB(testDB);
    Model model(testDB);

    SECTION("Insert and verify model") {
        ModelData newModel {1, "TestModel", "./path/to/cad/file", "{}", "Test Title", {}, "Author", "/file/path", "Library", true};
        REQUIRE(model.insertModel(newModel.id, newModel) == true);

        auto fetchedModel = model.getModelById(newModel.id);
        REQUIRE(fetchedModel.short_name == "TestModel");
        REQUIRE(fetchedModel.file_path == "/file/path");
        REQUIRE(fetchedModel.library_name == "Library");
    }

    SECTION("Update model attributes") {
        ModelData updatedModel {1, "UpdatedModel", "./new/path", "{\"updated\":true}", "Updated Title", {}, "New Author", "/new/file/path", "NewLibrary", false};
        REQUIRE(model.insertModel(updatedModel.id, updatedModel) == true);

        updatedModel.short_name = "ModifiedModel";
        REQUIRE(model.updateModel(updatedModel.id, updatedModel) == true);

        auto fetchedModel = model.getModelById(updatedModel.id);
        REQUIRE(fetchedModel.short_name == "ModifiedModel");
    }

    SECTION("Delete model") {
        ModelData deleteModel {2, "DeleteModel", "./delete/path", "{}", "Delete Title", {}, "Delete Author", "/delete/file/path", "DeleteLibrary", false};
        REQUIRE(model.insertModel(deleteModel.id, deleteModel) == true);
        REQUIRE(model.modelExists(deleteModel.id) == true);

        REQUIRE(model.deleteModel(deleteModel.id) == true);
        REQUIRE(model.modelExists(deleteModel.id) == false);
    }

    SECTION("Add and retrieve objects for model") {
        ModelData objModel {3, "ObjModel", "./obj/path", "{}", "Obj Title", {}, "Obj Author", "/obj/file/path", "ObjLibrary", false};
        REQUIRE(model.insertModel(objModel.id, objModel) == true);

        ObjectData obj1 {0, objModel.id, "Object1", -1, false};
        int obj1_id = model.insertObject(obj1);
        REQUIRE(obj1_id != -1);

        auto objects = model.getObjectsForModel(objModel.id);
        REQUIRE(objects.size() == 1);
        REQUIRE(objects[0].name == "Object1");
    }

    // Cleanup
    setupTestDB(testDB);
}

