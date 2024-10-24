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

# Add the test to CTest
add_test(NAME ModelTests COMMAND ModelTest)
