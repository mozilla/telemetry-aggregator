# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(telemetry-aggregator)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "telemetry-aggregator")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(externals)

find_package(LibLZMA REQUIRED)

include_directories(${LIBLZMA_INCLUDE_DIRS})

enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V)

# Build release with mkdir build; cd build; cmake -DCMAKE_BUILD_TYPE=Release ..

set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Ofast -flto")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fwhole-program")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")

# Add C++11 support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Ignore local unused typedefs, new in gcc 4.8, causes lots of warnings with
# rapidjson
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")

add_subdirectory(src)
add_subdirectory(dashboard)
add_subdirectory(tests)
