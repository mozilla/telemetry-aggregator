file(COPY result.txt result-1.txt result-2.txt DESTINATION .)

add_test(
  NAME merge-to-folder
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/merge-to-folder.sh"
)

add_test(
  NAME merge-two-to-folder
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/merge-two-to-folder.sh"
)

