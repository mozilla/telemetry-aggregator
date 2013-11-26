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

enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V)

set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Ofast -flto")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fwhole-program")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")

set(CMAKE_CXX_FLAGS "-std=c++11")

add_subdirectory(src)
add_subdirectory(tests)
