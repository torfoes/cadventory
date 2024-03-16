cmake_minimum_required(VERSION 3.25)

project(CADventory VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

# flag adjustments Qt requires
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /permissive-")
endif()

#find_package(SQLite3 REQUIRED)
include(cmake/sqlite.cmake)

# import the static library that was built
add_library(sqlite_static IMPORTED STATIC GLOBAL)
add_dependencies(sqlite_static sqlite)
set_target_properties(sqlite_static PROPERTIES
  IMPORTED_LOCATION "${sqlite_STATIC_LIBRARIES}"
)
add_custom_command(TARGET sqlite_static POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${sqlite_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include/sqlite
)

# All Warnings
if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

include_directories(${QT_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS})
set (SRCS
  src/CADventory.cpp
  src/FilesystemIndexer.cpp
  src/MainWindow.cpp
  src/SplashDialog.cpp
  src/Model.cpp
  src/main.cpp
  src/mainwindow.ui
  src/splash.ui
  src/Library.cpp
)
qt_add_executable(cadventory WIN32 ${SRCS})
add_dependencies(cadventory sqlite_static)

target_include_directories(cadventory PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cadventory PRIVATE Qt6::Core Qt6::Widgets sqlite_static)

# testing
enable_testing()
add_subdirectory(src/tests)

