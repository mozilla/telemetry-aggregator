file(COPY ss-ff-n-22.lzma ss-ff-n-28.lzma input.txt DESTINATION .)
file(COPY ss-ff-n-22-simple.lzma simple-input.txt test-simple.py DESTINATION .)
file(COPY corrupted.lzma corrupted-input.txt DESTINATION .)

add_test(
  NAME aggregator-test
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/aggregator-test.sh"
)

add_test(
  NAME simple-aggregator-test
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/simple-aggregator-test.sh"
)

add_test(
  NAME aggregator-corrupted-test
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/aggregator-corrupted-test.sh"
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_test(
    NAME aggregator-leak-test
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/aggregator-corrupted-test.sh"
  )
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("WARNING: Leak-tests disabled in release build, as release builds")
  message("         are statically linked!")
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")


