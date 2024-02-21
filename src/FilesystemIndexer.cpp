
#include "FilesystemIndexer.h"

#include <filesystem>


FilesystemIndexer::FilesystemIndexer(const char* rootDir) {
  indexDirectory(rootDir);
}


std::vector<std::string>
FilesystemIndexer::findFilesWithSuffixes(const std::vector<std::string>& suffixes) {
  std::vector<std::string> matchingFiles;
  for (const auto& suffix : suffixes) {
    auto it = fileIndex.find(suffix);
    if (it != fileIndex.end()) {
      matchingFiles.insert(matchingFiles.end(), it->second.begin(), it->second.end());
    }
  }
  return matchingFiles;
}


void
FilesystemIndexer::indexDirectory(const std::string& dir) {
  const std::filesystem::path& path = dir;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
    if (entry.is_regular_file()) {
      auto suffix = entry.path().extension().string();
      fileIndex[suffix].push_back(entry.path());
    }
  }
}


