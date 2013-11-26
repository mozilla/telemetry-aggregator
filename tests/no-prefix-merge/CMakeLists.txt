file(COPY result.txt DESTINATION .)

add_test(
  NAME no-merging-prefixes
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/no-merging-prefixes.sh"
)
