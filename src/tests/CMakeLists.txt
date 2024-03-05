
include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(FilesystemIndexerPerfTest FilesystemIndexerPerfTest.cpp ../FilesystemIndexer.cpp)

add_test(NAME FilesystemIndexerPerformanceTest COMMAND FilesystemIndexerPerfTest)


Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.3
)

FetchContent_MakeAvailable(Catch2)

add_executable(FilesystemIndexerTest FilesystemIndexerTest.cpp ../FilesystemIndexer.cpp)
target_link_libraries(FilesystemIndexerTest PRIVATE Catch2::Catch2WithMain)

add_test(NAME FilesystemIndexerTests COMMAND FilesystemIndexerTest)
