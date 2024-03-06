
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


TEST_CASE("ModelOperations", "[Model]") {
  std::string testDB = "test_models.db";
  setupTestDB(testDB); // Ensure we start with a fresh database

  SECTION("Create table and insert model") {
    Model model(testDB);

    bool insertResult = model.insertModel("testModel", "path/to/cad/file", "{}");
    REQUIRE(insertResult == true);
  }

  setupTestDB(testDB);
}
