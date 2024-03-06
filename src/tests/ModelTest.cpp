
/* let catch provide main() */
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "Model.h"
#include <filesystem>


void
setupTestDB(const std::string& path) {
  if (std::filesystem::exists(path)) {
    std::filesystem::remove(path);
  }
}


int
createTestModel(Model& model, const std::string& name = "TestModel", const std::string& cadFile = "path/to/cad/file", const std::string& overrideInfo = "{}") {
  model.insertModel(name, cadFile, overrideInfo);
  auto models = model.getModels();
  if (!models.empty()) {
    return models.front().id;
  }
  return -1; // oops.
}


TEST_CASE("ModelOperations", "[Model]") {
  std::string testDB = "test_models.db";
  setupTestDB(testDB); // Ensure we start with a fresh database
  Model model(testDB);

  SECTION("Insert model") {
    bool insertResult = model.insertModel("testModel", "path/to/cad/file", "{}");
    REQUIRE(insertResult == true);
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

    bool updateResult = model.updateModel(modelId, "updatedModel", "new/path/to/cad/file", "{\"updated\":true}");
    REQUIRE(updateResult == true);

    auto models = model.getModels();
    REQUIRE(models.size() == 1);
    REQUIRE(models.front().short_name == "updatedModel");
    REQUIRE(models.front().primary_file == "new/path/to/cad/file");
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
