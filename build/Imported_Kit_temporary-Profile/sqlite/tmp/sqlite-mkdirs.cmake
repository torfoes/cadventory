# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/anton/cadventory/build/Imported_Kit_temporary-Profile/sqlite/src/sqlite"
  "/home/anton/cadventory/build/Imported_Kit_temporary-Profile/sqlite/src/sqlite-build"
  "/home/anton/cadventory/build/Imported_Kit_temporary-Profile/sqlite/install"
  "/home/anton/cadventory/build/Imported_Kit_temporary-Profile/sqlite/tmp"
  "/home/anton/cadventory/build/Imported_Kit_temporary-Profile/sqlite/src/sqlite-stamp"
  "/home/anton/cadventory/build/Imported_Kit_temporary-Profile/sqlite/src"
  "/home/anton/cadventory/build/Imported_Kit_temporary-Profile/sqlite/src/sqlite-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/anton/cadventory/build/Imported_Kit_temporary-Profile/sqlite/src/sqlite-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/anton/cadventory/build/Imported_Kit_temporary-Profile/sqlite/src/sqlite-stamp${cfgdir}") # cfgdir has leading slash
endif()
