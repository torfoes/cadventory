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
install(FILES splash.png
  # eventually should go to DATADIR
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES doc/CADventory.docx
  DESTINATION ${CMAKE_INSTALL_DOCDIR}
)


# windows insanely wants DLLs in same folder as the app so set it up
if(WIN32)
  # function to copy Qt DLLs for a given target
  function (copy_qt_dlls target)
    foreach (qt_module IN ITEMS ${LIBS})
      if (CMAKE_BUILD_TYPE MATCHES "Debug")
	get_target_property(lib_location ${qt_module} IMPORTED_LOCATION_DEBUG)
      else ()
	get_target_property(lib_location ${qt_module} IMPORTED_LOCATION_RELEASE)
      endif ()

      if (lib_location)
        message("Copying DLLs from ${lib_location}")

        # assume DLL is in dir relative to the .lib file
        get_filename_component(lib_dir ${lib_location} DIRECTORY)
        get_filename_component(lib_name ${lib_location} NAME_WE)

	# remove 'lib' prefix if present
        string(REGEX REPLACE "^lib" "" dll_name ${lib_name}) 
        set(dll_path "${lib_dir}/../bin/${dll_name}.dll")

        # copy DLL to the target output dir
        add_custom_command(TARGET ${target} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll_path}" $<TARGET_FILE_DIR:${target}>
          COMMENT "Copying ${dll_path} to $<TARGET_FILE_DIR:${target}>"
        )
      else ()
        message(WARNING "DLL for ${qt_module} not found.")
      endif ()
    endforeach ()
  endfunction ()

  # Call the function to copy the Qt DLLs for your target
  copy_qt_dlls(cadventory)
endif ()

# also copy Qt plugins.  probably a better way, but this will do the trick for now.
if(WIN32)
  set(QT_PLUGINS_DIR "${Qt6_DIR}/../../../plugins")
  set(QT_REQUIRED_PLUGINS platforms imageformats)

  foreach(plugin ${QT_REQUIRED_PLUGINS})
    set(PLUGIN_SRC_DIR "${QT_PLUGINS_DIR}/${plugin}")
    set(PLUGIN_DST_DIR "$<TARGET_FILE_DIR:cadventory>/${plugin}")

    # opy the plugin directory to the application dir
    add_custom_command(TARGET cadventory POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGIN_DST_DIR}
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${PLUGIN_SRC_DIR} ${PLUGIN_DST_DIR}
      COMMENT "Copying Qt plugin: ${plugin}\n"
    )
  endforeach()
endif()


# testing
enable_testing()
add_subdirectory(src/tests)

