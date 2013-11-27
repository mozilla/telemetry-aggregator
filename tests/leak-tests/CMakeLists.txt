file(COPY result.txt DESTINATION .)

add_test(
  NAME merge-leak-test
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/merge-leak-test.sh"
)
