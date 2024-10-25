
/* let catch provide main() */
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "Model.h"

void setupTestDB(const std::string& path) {
  if (std::filesystem::exists(path)) {
    std::filesystem::remove(path);
  }
}

int createTestModel(Model& model, const std::string& name = "TestModel",
                    const std::string& cadFile = "./truck.g",
                    const std::string& overrideInfo = "{}") {
  // model.insertModel(cadFile, name, "primary", overrideInfo);
  auto models = model.getModels();
  if (!models.empty()) {
    return models.front().id;
  }
  return -1;  // oops.
}

TEST_CASE("ModelOperations", "[Model]") {
  std::string testDB = "test_models.db";
  setupTestDB(testDB);
  Model model(testDB);

  SECTION("Insert model") {
    // bool insertResult = model.insertModel("path/to/cad/file", "testModel",
    //                                       "primary", "{\"test\":true}");
    // REQUIRE(insertResult == true);
  }

  SECTION("Read model") {
    int modelId = createTestModel(model);
    REQUIRE(modelId != -1);

    auto models = model.getModels();
    REQUIRE(models.size() == 1);
    REQUIRE(models.front().id == modelId);
  }

  SECTION("Update model") {
    int modelId = createTestModel(model);
    REQUIRE(modelId != -1);

    bool updateResult = model.updateModel(
        modelId, "updatedModel", "new/path/to/cad/file", "new/path/to/cad/file", "library", "{\"updated\":true}");
    REQUIRE(updateResult == true);

    auto models = model.getModels();
    REQUIRE(models.size() == 1);
    REQUIRE(models.front().short_name == "updatedModel");
    REQUIRE(models.front().primary_file_path == "new/path/to/cad/file");
    REQUIRE(models.front().override_info == "{\"updated\":true}");
  }

  SECTION("Delete model") {
    int modelId = createTestModel(model);
    REQUIRE(modelId != -1);

    bool deleteResult = model.deleteModel(modelId);
    REQUIRE(deleteResult == true);

    auto models = model.getModels();
    REQUIRE(models.empty() == true);
  }

  setupTestDB(testDB);
}

TEST_CASE("TagOperations", "[Model]") {
  std::string testDB = "test_tags.db";
  setupTestDB(testDB);
  Model model(testDB);

  SECTION("Add tag to model") {
    int modelId = createTestModel(model);
    REQUIRE(modelId != -1);

    bool addResult = model.addTagToModel(modelId, "testTag");
    REQUIRE(addResult == true);

    auto tags = model.getTagsForModel(modelId);
    REQUIRE(tags.size() == 1);
    REQUIRE(tags.front() == "testTag");
  }

  SECTION("Remove tag from model") {
    int modelId = createTestModel(model);
    REQUIRE(modelId != -1);

    model.addTagToModel(modelId, "testTag");
    auto tags = model.getTagsForModel(modelId);
    REQUIRE(tags.size() == 1);

    bool removeResult = model.removeTagFromModel(modelId, "testTag");
    REQUIRE(removeResult == true);

    tags = model.getTagsForModel(modelId);
    REQUIRE(tags.empty() == true);
  }

  SECTION("Get all tags from a model") {
    int modelId = createTestModel(model);
    REQUIRE(modelId != -1);

    model.addTagToModel(modelId, "tag1");
    model.addTagToModel(modelId, "tag2");
    model.addTagToModel(modelId, "tag3");

    auto tags = model.getTagsForModel(modelId);
    REQUIRE(tags.size() == 3);
    REQUIRE(tags[0] == "tag1");
    REQUIRE(tags[1] == "tag2");
    REQUIRE(tags[2] == "tag3");
  }

  setupTestDB(testDB);
}
