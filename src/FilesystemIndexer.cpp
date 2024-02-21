
#include "FilesystemIndexer.h"

#include <filesystem>
#include <iostream>


FilesystemIndexer::FilesystemIndexer(const char* rootDir) {
  indexDirectory(rootDir);
}


std::vector<std::string>
FilesystemIndexer::findFilesWithSuffixes(const std::vector<std::string>& suffixes) {
  std::vector<std::string> matchingFiles;
  for (const auto& suffix : suffixes) {
    std::cout << "looking for " << suffix << std::endl;
    auto it = fileIndex.find(suffix);
    if (it != fileIndex.end()) {
      matchingFiles.insert(matchingFiles.end(), it->second.begin(), it->second.end());
    }
  }
  return matchingFiles;
}


void
FilesystemIndexer::indexDirectory(const std::string& dir) {
  std::filesystem::path path = dir;
  std::filesystem::recursive_directory_iterator it(path), end;

  while (it != end) {
    try {
      if (it->is_regular_file()) {
        auto suffix = it->path().extension().string();
        fileIndex[suffix].push_back(it->path());
      }
      ++it;
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Warning: Skipped directory due to permissions - " << e.what() << std::endl;
      it.disable_recursion_pending();
      // ++it; already skipped for us.
    }
  }
}


size_t
FilesystemIndexer::indexed() {
  return fileIndex.size();
}
