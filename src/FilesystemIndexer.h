#ifndef FILESYSTEMINDEXER_H
#define FILESYSTEMINDEXER_H

#include <unordered_map>
#include <vector>
#include <string>


class FilesystemIndexer {

public:
  explicit FilesystemIndexer(const char* rootDir);

  std::vector<std::string> findFilesWithSuffixes(const std::vector<std::string>& suffixes);

private:
  std::unordered_map<std::string, std::vector<std::string>> fileIndex;

  void indexDirectory(const std::string& path);
};


#endif /* FILESYSTEMINDEXER_H */
