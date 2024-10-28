# src/tests/CMakeLists.txt

cmake_minimum_required(VERSION 3.25)

project(CADventoryTests)

# Include the parent source directory to access headers
include_directories(${CMAKE_SOURCE_DIR}/src)

# Find the required Qt modules
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)

# Include FetchContent module
include(FetchContent)

# Fetch Catch2 testing framework
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.3
)
FetchContent_MakeAvailable(Catch2)

# Add compiler flags for code coverage if in Debug mode
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Adding code coverage flags")
    add_compile_options(-fprofile-arcs -ftest-coverage -g -O0)
    link_libraries(gcov)  # gcov library for coverage
endif()

# Add the ModelTest executable
add_executable(ModelTest ModelTest.cpp ../Model.cpp)

# Link the test executable against the necessary libraries
target_link_libraries(ModelTest PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Gui
    ${SQLITE_LIBRARY}
)

# Include directories for SQLite (if necessary)
target_include_directories(ModelTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_BINARY_DIR}/../..
)

# Add the FilesystemIndexerTest executable
add_executable(FilesystemIndexerTest FilesystemIndexerTest.cpp ../FilesystemIndexer.cpp)

# Link the FilesystemIndexerTest executable against the necessary libraries
target_link_libraries(FilesystemIndexerTest PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Gui
    ${SQLITE_LIBRARY}
)

# Include directories for FilesystemIndexerTest if necessary
target_include_directories(FilesystemIndexerTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_BINARY_DIR}/../..
)

# Add the FilesystemIndexerPerfTest executable
add_executable(FilesystemIndexerPerfTest FilesystemIndexerPerfTest.cpp ../FilesystemIndexer.cpp)

# Link the FilesystemIndexerPerfTest executable against the necessary libraries
target_link_libraries(FilesystemIndexerPerfTest PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Gui
    ${SQLITE_LIBRARY}
)

# Include directories for FilesystemIndexerPerfTest if necessary
target_include_directories(FilesystemIndexerPerfTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_BINARY_DIR}/../..
)

# Register each test with CTest
add_test(NAME ModelTests COMMAND ModelTest)
add_test(NAME FilesystemIndexerTests COMMAND FilesystemIndexerTest)
add_test(NAME FilesystemIndexerPerfTests COMMAND FilesystemIndexerPerfTest)
