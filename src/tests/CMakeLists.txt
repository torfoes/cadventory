cmake_minimum_required(VERSION 3.10)
project(FilesystemIndexerTest)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(FilesystemIndexerTest FilesystemIndexerTest.cpp ../FilesystemIndexer.cpp)

add_test(NAME FilesystemIndexerPerformanceTest COMMAND FilesystemIndexerTest)
