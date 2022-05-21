# uinputcpp
# Copyright (C) 2022 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.0)
project(uinput VERSION 0.1.0)

set(TINYCMMC_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external/cmake-modules/modules/")
find_package(tinycmmc CONFIG)
message(STATUS "tinycmmc module path: ${TINYCMMC_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH ${TINYCMMC_MODULE_PATH})

include(GNUInstallDirs)
# include(MaximumWarnings)
# include(ClangTidy)

find_package(PkgConfig REQUIRED)
pkg_search_module(DBUSGLIB REQUIRED dbus-glib-1 IMPORTED_TARGET)

# Build dependencies
function(build_dependencies)
  set(BUILD_TESTS OFF)

  find_package(strutcpp QUIET)
  if(NOT TARGET strut)
    add_subdirectory(external/strutcpp/ EXCLUDE_FROM_ALL)
  endif()

  find_package(logmich QUIET)
  if(NOT TARGET logmich::logmich)
    add_subdirectory(external/logmich/ EXCLUDE_FROM_ALL)
  endif()
endfunction()
build_dependencies()

file(GLOB UINPUT_SOURCES src/*.cpp)
file(GLOB UINPUT_HEADER_SOURCES include/uinput/*.hpp)
add_library(uinput STATIC ${UINPUT_SOURCES})
set_target_properties(uinput PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
set_target_properties(uinput PROPERTIES PUBLIC_HEADER "${UINPUT_HEADER_SOURCES}")
target_compile_options(uinput PRIVATE ${WARNINGS_CXX_FLAGS})
target_link_libraries(uinput PUBLIC
  logmich::logmich
  PkgConfig::DBUSGLIB
  )
target_include_directories(uinput SYSTEM PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

if(BUILD_TESTS)
  find_package(GTest REQUIRED)

  # build uinput tests
  file(GLOB TEST_UINPUT_SOURCES tests/*.cpp)
  add_executable(test_uinput ${TEST_UINPUT_SOURCES})
  set_target_properties(test_uinput PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
  target_compile_options(test_uinput PRIVATE ${WARNINGS_CXX_FLAGS})
  target_include_directories(test_uinput PUBLIC src/)
  target_link_libraries(test_uinput
    GTest::GTest
    GTest::Main
    uinput)

  # add 'make test' target, use 'make test ARGS="-V"' or 'ctest -V' for verbose
  enable_testing()
  add_test(NAME test_uinput
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND test_uinput)
endif()

include(ExportAndInstallLibrary)

# EOF #