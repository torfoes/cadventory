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

qt6_wrap_ui(UIS_HEADERS ../src/mainwindow.ui ../src/librarywindow.ui ../src/reportgenerationwindow.ui ../src/SettingWindow.ui ../src/modelview.ui)

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

# Add the LibraryTest executable
add_executable(LibraryTest
    LibraryTest.cpp
    ../Library.cpp
    ../Model.cpp
    ../FilesystemIndexer.cpp
)

# Link the test executable against the necessary libraries
target_link_libraries(LibraryTest PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Gui
    ${SQLITE_LIBRARY}
)

# Include directories for LibraryTest
target_include_directories(LibraryTest PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
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
    ../ModelFilterProxyModel.cpp
    ../mainwindow.ui
    ../ReportGeneratorWorker.cpp
    ../ReportGeneratorWorker.h
    ../SettingWindow.h
    ../SettingWindow.cpp
    ../FileSystemModelWithCheckboxes.cpp
    ../FileSystemModelWithCheckboxes.h
    ../FileSystemFilterProxyModel.cpp
    ../FileSystemFilterProxyModel.h
    ../ModelView.cpp
    ../ModelView.h
    ../modelview.ui
)

# Link the Main Window GUI test executable against the necessary libraries
# target_link_libraries(MainWindowTest PRIVATE
#     Catch2::Catch2WithMain
#     Qt6::Core
#     Qt6::Gui
#     Qt6::Widgets
#     Qt6::Test
#     ${SQLITE_LIBRARY}
# )

# Include directories for MainWindowTest
# target_include_directories(MainWindowTest PRIVATE
#     ${CMAKE_SOURCE_DIR}/src
#     ${CMAKE_BINARY_DIR}/src
# )

# Add the LibraryWindowTest executable
# add_executable(LibraryWindowTest
#     LibraryWindowTest.cpp
#     ../LibraryWindow.cpp
#     ../Library.cpp
#     ../Model.cpp
#     ../ProcessGFiles.cpp
#     ../IndexingWorker.cpp
#     ../FilesystemIndexer.cpp
#     ../ModelCardDelegate.cpp
#     ../GeometryBrowserDialog.cpp
#     ../ReportGenerationWindow.cpp
#     ../ModelFilterProxyModel.cpp
#     ../mainwindow.ui
#     ../librarywindow.ui
# )

# Link the LibraryWindow GUI test executable against the necessary libraries
# target_link_libraries(LibraryWindowTest PRIVATE
#     Catch2::Catch2WithMain
#     Qt6::Core
#     Qt6::Gui
#     Qt6::Widgets
#     Qt6::Test
#     ${SQLITE_LIBRARY}
# )

# Include directories for LibraryWindowTest
# target_include_directories(LibraryWindowTest PRIVATE
#     ${CMAKE_SOURCE_DIR}/src
#     ${CMAKE_BINARY_DIR}/src
# )

# Add the GeometryBrowserDialogTest executable
add_executable(GeometryBrowserDialogTest
    GeometryBrowserDialogTest.cpp
    ../GeometryBrowserDialog.cpp
    ../Model.cpp
)

# Link the GeometryBrowserDialogTest executable against the necessary libraries
target_link_libraries(GeometryBrowserDialogTest PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
    ${SQLITE_LIBRARY}
)

# Include directories for GeometryBrowserDialogTest
target_include_directories(GeometryBrowserDialogTest PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
)

# Add the ProcessGFilesTest executable
add_executable(ProcessGFilesTest
    ProcessGFilesTest.cpp
    ../ProcessGFiles.cpp
    ../Model.cpp
)

# Link the test executable against the necessary libraries
target_link_libraries(ProcessGFilesTest PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Gui
    ${SQLITE_LIBRARY}
)

# Add the IndexingWorkerTest executable
add_executable(IndexingWorkerTest
    IndexingWorkerTest.cpp
    ../IndexingWorker.cpp
    ../Library.cpp
    ../Model.cpp
    ../ProcessGFiles.cpp          
    ../FilesystemIndexer.cpp
)

# Link the test executable against the necessary libraries
target_link_libraries(IndexingWorkerTest PRIVATE
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    sqlite3
)


# Register each test with CTest
add_test(NAME ModelTests COMMAND ModelTest)
add_test(NAME LibraryTests COMMAND LibraryTest)
add_test(NAME FilesystemIndexerTests COMMAND FilesystemIndexerTest)
add_test(NAME FilesystemIndexerPerfTests COMMAND FilesystemIndexerPerfTest)
# add_test(NAME MainWindowTests COMMAND MainWindowTest)
# add_test(NAME LibraryWindowTests COMMAND LibraryWindowTest)
add_test(NAME GeometryBrowserDialogTests COMMAND GeometryBrowserDialogTest)
add_test(NAME ProcessGFilesTests COMMAND ProcessGFilesTest)
add_test(NAME IndexingWorkerTests COMMAND IndexingWorkerTest)
