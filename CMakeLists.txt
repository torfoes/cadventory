cmake_minimum_required(VERSION 3.25)

project(CADventory VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

include_directories(${QT_INCLUDE_DIRS})
set (SRCS
  src/CADventory.cpp
  src/FilesystemIndexer.cpp
  src/MainWindow.cpp
  src/SplashDialog.cpp
  src/main.cpp
  src/mainwindow.ui
  src/splash.ui
  src/Library.cpp
)
qt_add_executable(cadventory WIN32 ${SRCS})

target_link_libraries(cadventory PRIVATE Qt6::Core Qt6::Widgets)
