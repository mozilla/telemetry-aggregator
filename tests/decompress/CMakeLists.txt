file(COPY a-b-c-def-g.txt.xz a-b-c-def-g.txt.lzma DESTINATION .)

set(COMPRESSEDFILEREADERTEST_SRC
    CompressedFileReaderTest.cpp
    ../../src/CompressedFileReader.cpp
)

include_directories(${LIBLZMA_INCLUDE_DIRS})
add_executable(compressedfilereadertest ${COMPRESSEDFILEREADERTEST_SRC})
target_link_libraries(compressedfilereadertest ${LIBLZMA_LIBRARIES})

add_test(
  NAME CompressedFileReaderTest
  COMMAND ./compressedfilereadertest
)
