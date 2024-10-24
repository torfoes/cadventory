#include <cassert>
#include <chrono>
#include <iostream>

#include "FilesystemIndexer.h"

void testIndexDirectoryPerformance() {
  FilesystemIndexer indexer;

  // once to prime
  size_t files = indexer.indexDirectory("/");

  auto start = std::chrono::high_resolution_clock::now();
  files = indexer.indexDirectory("/");
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end - start;
  auto rate = files / (duration.count() / 1000.0);
  std::cout << "Index Rate is " << rate << " files/sec" << std::endl;
  std::cout << "Indexing " << files << " files took " << duration.count()
            << " ms" << std::endl;

  // Check if the indexing meets our performance criteria
  assert(rate > 10000);  // 10k files/sec
}

void testFindFilesWithSuffixesPerformance() {
  // Assuming the indexer has been populated with a large dataset
  FilesystemIndexer indexer;
  indexer.indexDirectory(".");

  std::vector<std::string> suffixes = {".cpp", ".h", ".png", ".jpg", ".txt"};

  auto start = std::chrono::high_resolution_clock::now();
  auto files = indexer.findFilesWithSuffixes(suffixes);
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end - start;
  auto rate = files.size() / (duration.count() / 1000.0);
  std::cout << "Find rate is " << rate << " files/sec" << std::endl;
  std::cout << "Finding files with given suffixes took " << duration.count()
            << " ms" << std::endl;

  // Check if file finding meets our criteria
  assert(rate > 100000);  // 100k files/sec
}

int main() {
  testIndexDirectoryPerformance();
  testFindFilesWithSuffixesPerformance();

  return 0;
}
