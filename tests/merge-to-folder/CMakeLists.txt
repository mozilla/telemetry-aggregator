file(COPY result.txt result-1.txt result-2.txt DESTINATION .)

add_test(
  NAME merge-to-folder
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/merge-to-folder.sh"
)

add_test(
  NAME merge-two-to-folder
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/merge-two-to-folder.sh"
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_test(
    NAME merge-to-folder-leak-test
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/merge-to-folder-leak-test.sh"
  )
else (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("WARNING: Leak-tests disabled in release build, as release builds")
  message("         are statically linked!")
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
