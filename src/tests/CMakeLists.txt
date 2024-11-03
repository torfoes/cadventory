# src/tests/CMakeLists.txt

cmake_minimum_required(VERSION 3.25)

project(CADventoryTests)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
)

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

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Adding code coverage flags")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC-specific coverage flags
        add_compile_options(-fprofile-arcs -ftest-coverage -g -O0)
        link_libraries(gcov)  # gcov library for coverage
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Clang-specific coverage flags
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping -g -O0)
        add_link_options(-fprofile-instr-generate)
    else()
        message(WARNING "Code coverage is not supported for this compiler.")
    endif()
endif()

# Add the ModelTest executable
add_executable(ModelTest
    ModelTest.cpp
    ../Model.cpp
)

# Link the test executable against the necessary libraries
target_link_libraries(ModelTest PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Gui
    ${SQLITE_LIBRARY}
)

# Include directories for ModelTest
target_include_directories(ModelTest PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
)


# Add the MainWindowTest executable
add_executable(MainWindowTest
    MainWindowTest.cpp
    ../MainWindow.cpp
    ../LibraryWindow.cpp
    ../Library.cpp
    ../Model.cpp
    ../ProcessGFiles.cpp
    ../IndexingWorker.cpp
    ../FilesystemIndexer.cpp
    ../ModelCardDelegate.cpp
    ../GeometryBrowserDialog.cpp
    ../ReportGenerationWindow.cpp
    ../librarywindow.ui
    ../mainwindow.ui
    ../reportgenerationwindow.ui
    ../ModelFilterProxyModel.cpp
    ../ReportGeneratorWorker.cpp
    ../ReportGeneratorWorker.h
)

# Link the Main Window GUI test executable against the necessary libraries
target_link_libraries(MainWindowTest PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
    ${SQLITE_LIBRARY}
)

# Include directories for MainWindowTest
target_include_directories(MainWindowTest PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
)

# Register each test with CTest
add_test(NAME ModelTests COMMAND ModelTest)
add_test(NAME LibraryTests COMMAND LibraryTest)
# Add other tests as needed
add_test(NAME MainWindowTests COMMAND MainWindowTest)
