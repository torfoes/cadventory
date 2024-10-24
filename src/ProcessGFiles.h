#ifndef PROCESS_G_FILES_H
#define PROCESS_G_FILES_H

#include <sqlite3.h>

#include <filesystem>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

class ProcessGFiles {
 public:
  ProcessGFiles();
  std::map<std::string, std::string> processGFile(
      const std::filesystem::path& file_path);
  std::map<std::string, std::string> executeMultiThreadedProcessing(
      const std::vector<std::string>& allGeometry, int num_workers = 4);
  std::pair<std::string, std::string> runCommand(const std::string& command);

 private:
  std::string dbPath;
  void gFileWorker(std::queue<std::filesystem::path>& file_queue);
};

#endif  // PROCESS_G_FILES_H
