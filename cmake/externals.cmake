# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(ExternalProject)
set_property(DIRECTORY PROPERTY EP_BASE "${CMAKE_BINARY_DIR}/externals")

externalproject_add(
    rapidjson
    SVN_REPOSITORY      http://rapidjson.googlecode.com/svn/trunk/
    SVN_REVISION        "-r131"
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ""
)

set(RAPIDJSON_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/externals/Source/rapidjson/include")
include_directories(${RAPIDJSON_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${RAPIDJSON_INCLUDE_DIRS}")
