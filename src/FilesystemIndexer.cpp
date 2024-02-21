
#include "FilesystemIndexer.h"

#include <filesystem>
#include <iostream>


FilesystemIndexer::FilesystemIndexer(const char* rootDir) : callback(nullptr) {
  if (rootDir) {
    indexDirectory(rootDir);
  }
}


FilesystemIndexer::~FilesystemIndexer() {
  fileIndex.clear();
  visitedPaths.clear();
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
FilesystemIndexer::setProgressCallback(std::function<void(const std::string&)> func) {
  callback = func;
}


size_t
FilesystemIndexer::indexDirectory(const std::string& dir) {

  if (dir == "")
    return 0;

  std::filesystem::path path = dir;
  // resolve symbolic links
  auto normalized = std::filesystem::canonical(dir).string();

  // avoid cyclic references
  if (!visitedPaths.insert(normalized).second) {
    return 0;
  }

  size_t count = 0;

  try {
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
      try {
        bool isReadable = (entry.status().permissions() & std::filesystem::perms::owner_read) != std::filesystem::perms::none;
        if (!isReadable)
          continue;

        if (std::filesystem::is_directory(entry.status())) {
          // recurse
          count += indexDirectory(entry.path());
        } else if (std::filesystem::is_regular_file(entry.status())) {
          auto suffix = entry.path().extension().string();
          fileIndex[suffix].push_back(entry.path().string());
          count++;

          if (callback)
            callback(std::string("Indexing ") + entry.path().string());
        }
      } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "WARNING: Unable to access " << entry.path() << " - " << e.what() << std::endl;
      }
    }
  } catch (const std::filesystem::filesystem_error& e) {
    // handle fs security and/or attributes silently for now..
    // std::cerr << "WARNING: Skipping " << dir << " - " << e.what() << std::endl;
  }

  return count;
}


#if 0
void
FilesystemIndexer::indexDirectory(const std::string& dir) {
  std::filesystem::path path = dir;
  std::filesystem::recursive_directory_iterator it(path), end;
  size_t count = 0;
  while (it != end) {
    try {
      auto status = std::filesystem::status(*it);

      bool isReadable = (status.permissions() & std::filesystem::perms::owner_read) != std::filesystem::perms::none;

      if (it->path() == "/Users/morrison/Library/Application Support/MobileSync") {
        std::cout << "ENCOUNTERED " << isReadable << std::endl;
        it.disable_recursion_pending();
        ++it;
        continue;
      }

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
#endif


size_t
FilesystemIndexer::indexed() {
  size_t total = 0;
  for (const auto& itr : fileIndex) {
    total += itr.second.size();
  }
  return total;
}
