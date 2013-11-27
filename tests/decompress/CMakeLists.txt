file(COPY a-b-c-def-g.txt.xz a-b-c-def-g.txt.lzma DESTINATION .)
file(COPY aaa-bbb-ccc-ddd-eee.txt.xz aaa-bbb-ccc-ddd-eee.txt.lzma DESTINATION .)

# CompressedFileReaderTest
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

# LargeCompressedFileReaderTest
set(LARGECOMPRESSEDFILEREADERTEST_SRC
    LargeCompressedFileReaderTest.cpp
    ../../src/CompressedFileReader.cpp
)

include_directories(${LIBLZMA_INCLUDE_DIRS})
add_executable(largecompressedfilereadertest ${COMPRESSEDFILEREADERTEST_SRC})
target_link_libraries(largecompressedfilereadertest ${LIBLZMA_LIBRARIES})

add_test(
  NAME LargeCompressedFileReaderTest
  COMMAND ./largecompressedfilereadertest
)