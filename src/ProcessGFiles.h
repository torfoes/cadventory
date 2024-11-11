#ifndef PROCESSGFILES_H
#define PROCESSGFILES_H

#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <thread>
#include <vector>

#include "Model.h"
#include "config.h"

class ProcessGFiles {
 public:
  explicit ProcessGFiles(Model* model, bool debug = false);

  std::tuple<bool, std::string> generateGistReport(
      const std::string& inputFilePath, const std::string& outputFilePath,
      const std::string& primary_obj);

  void processGFile(const std::filesystem::path& file_path,
                    const std::string& previews_folder,
                    const std::string& library_name = "(unknown)");

 private:
  // Helper methods
  void debugPrint(const std::string& message);
  void debugError(const std::string& message);
  bool isModelProcessed(int modelId);
  void extractTitle(ModelData& modelData, const std::string& file_path);

  std::vector<ObjectData> extractObjects(
      const ModelData& modelData, const std::string& file_path,
      std::map<std::string, std::string>& parentRelations);

  void generateThumbnail(ModelData& modelData, const std::string& file_path,
                         const std::string& previews_folder,
                         const std::string& selected_object_name);

  // Command execution helpers
  std::tuple<std::string, std::string, int> runCommand(
      const std::string& command, int timeout_seconds = 10);
  std::vector<std::string> parseTopsOutput(const std::string& tops_output);
  std::vector<std::string> parseLtOutput(const std::string& lt_output);
  bool validateObject(const std::string& file_path,
                      const std::string& object_name);
  std::vector<std::string> splitStringByWhitespace(const std::string& input);

  Model* model;
  bool debug;
  std::mutex db_mutex;
};

#endif  // PROCESSGFILES_H
