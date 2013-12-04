#.rst:
# FindLibLZMA
# -----------
#
# Find LibLZMA
#
# Find LibLZMA headers and library
#
# ::
#
#   LIBLZMA_FOUND             - True if liblzma is found.
#   LIBLZMA_INCLUDE_DIRS      - Directory where liblzma headers are located.
#   LIBLZMA_LIBRARIES         - Lzma libraries to link against.
#   LIBLZMA_HAS_AUTO_DECODER  - True if lzma_auto_decoder() is found (required).
#   LIBLZMA_HAS_EASY_ENCODER  - True if lzma_easy_encoder() is found (required).
#   LIBLZMA_HAS_LZMA_PRESET   - True if lzma_lzma_preset() is found (required).
#   LIBLZMA_VERSION_MAJOR     - The major version of lzma
#   LIBLZMA_VERSION_MINOR     - The minor version of lzma
#   LIBLZMA_VERSION_PATCH     - The patch version of lzma
#   LIBLZMA_VERSION_STRING    - version number as a string (ex: "5.0.3")

#=============================================================================
# Copyright 2008 Per Øyvind Karlsen <peroyvind@mandriva.org>
# Copyright 2009 Alexander Neundorf <neundorf@kde.org>
# Copyright 2009 Helio Chissini de Castro <helio@kde.org>
# Copyright 2012 Mario Bensi <mbensi@ipsquad.net>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# 
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================


find_path(LIBLZMA_INCLUDE_DIR lzma.h )
find_library(LIBLZMA_LIBRARY lzma)

if(LIBLZMA_INCLUDE_DIR AND EXISTS "${LIBLZMA_INCLUDE_DIR}/lzma/version.h")
    file(STRINGS "${LIBLZMA_INCLUDE_DIR}/lzma/version.h" LIBLZMA_HEADER_CONTENTS REGEX "#define LZMA_VERSION_[A-Z]+ [0-9]+")

    string(REGEX REPLACE ".*#define LZMA_VERSION_MAJOR ([0-9]+).*" "\\1" LIBLZMA_VERSION_MAJOR "${LIBLZMA_HEADER_CONTENTS}")
    string(REGEX REPLACE ".*#define LZMA_VERSION_MINOR ([0-9]+).*" "\\1" LIBLZMA_VERSION_MINOR "${LIBLZMA_HEADER_CONTENTS}")
    string(REGEX REPLACE ".*#define LZMA_VERSION_PATCH ([0-9]+).*" "\\1" LIBLZMA_VERSION_PATCH "${LIBLZMA_HEADER_CONTENTS}")

    set(LIBLZMA_VERSION_STRING "${LIBLZMA_VERSION_MAJOR}.${LIBLZMA_VERSION_MINOR}.${LIBLZMA_VERSION_PATCH}")
    unset(LIBLZMA_HEADER_CONTENTS)
endif()

# We're using new code known now as XZ, even library still been called LZMA
# it can be found in http://tukaani.org/xz/
# Avoid using old codebase
if (LIBLZMA_LIBRARY)
   include(CheckLibraryExists)
   CHECK_LIBRARY_EXISTS(${LIBLZMA_LIBRARY} lzma_auto_decoder "" LIBLZMA_HAS_AUTO_DECODER)
   CHECK_LIBRARY_EXISTS(${LIBLZMA_LIBRARY} lzma_easy_encoder "" LIBLZMA_HAS_EASY_ENCODER)
   CHECK_LIBRARY_EXISTS(${LIBLZMA_LIBRARY} lzma_lzma_preset "" LIBLZMA_HAS_LZMA_PRESET)
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibLZMA  REQUIRED_VARS  LIBLZMA_INCLUDE_DIR
                                                          LIBLZMA_LIBRARY
                                                          LIBLZMA_HAS_AUTO_DECODER
                                                          LIBLZMA_HAS_EASY_ENCODER
                                                          LIBLZMA_HAS_LZMA_PRESET
                                           VERSION_VAR    LIBLZMA_VERSION_STRING
                                 )

if (LIBLZMA_FOUND)
    set(LIBLZMA_LIBRARIES ${LIBLZMA_LIBRARY})
    set(LIBLZMA_INCLUDE_DIRS ${LIBLZMA_INCLUDE_DIR})
endif ()

mark_as_advanced( LIBLZMA_INCLUDE_DIR LIBLZMA_LIBRARY )
