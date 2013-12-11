file(COPY result.txt DESTINATION .)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_test(
    NAME merge-leak-test
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/merge-leak-test.sh"
  )
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("WARNING: Leak-tests disabled in release build, as release builds")
  message("         are statically linked!")
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

