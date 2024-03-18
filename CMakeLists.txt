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

# support sqlite built from external or against system
if (DEFINED CADVENTORY_USE_SQLITE)
  find_package(SQLite3 REQUIRED)
  set (SQLITE_LIBRARY sqlite3)
else()
  include(cmake/sqlite.cmake)
  set (SQLITE_LIBRARY sqlite_static)
endif()

# import the static library that was built
add_library(sqlite_static IMPORTED STATIC GLOBAL)
add_dependencies(sqlite_static sqlite)
set_target_properties(sqlite_static PROPERTIES
  IMPORTED_LOCATION "${sqlite_STATIC_LIBRARIES}"
)

# all warnings for compliance
if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

# build CADventory
include_directories(${QT_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS} ${sqlite_INCLUDE_DIR})
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

# note dependencies
add_dependencies(cadventory sqlite_static)
target_include_directories(cadventory PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(cadventory PRIVATE ${CMAKE_CURRENT_BUILD_DIR}/sqlite/install/include)
target_link_libraries(cadventory PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui ${SQLITE_LIBRARY}
)

# deployment on Windows needs DLLs copied
install(TARGETS cadventory
  BUNDLE  DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (WIN32)
  qt_generate_deploy_app_script(
    TARGET cadventory
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
  )
  install(SCRIPT ${deploy_script})
endif (WIN32)


# testing
enable_testing()
add_subdirectory(src/tests)

