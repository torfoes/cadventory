
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
  size_t count = 0;
  while (it != end) {
    try {
      auto status = std::filesystem::status(*it);
      bool isReadable = (status.permissions() & std::filesystem::perms::owner_read) != std::filesystem::perms::none;

      if (isReadable) {
        if (it->is_regular_file()) {
          auto suffix = it->path().extension().string();
          fileIndex[suffix].push_back(it->path());
        } else {
          count++;
        }
      } else {
        std::cerr << "Skipping due to insufficient permissions: " << it->path() << std::endl;
        it.disable_recursion_pending();
      }
      ++it;
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Warning: Skipped directory due to permissions - " << e.what() << std::endl;
      it.disable_recursion_pending();

      /* try to continue, but may be invalidated */
      if (it != end)
        ++it;
    }
  }
  std::cout << "dir count is " << count << std::endl;
}


size_t
FilesystemIndexer::indexed() {
  return fileIndex.size();
}
