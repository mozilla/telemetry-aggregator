file(COPY result-1.txt result-2.txt unsorted-results.txt  DESTINATION .)

add_test(
  NAME merge-test-with-options
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/with-options.sh"
)

add_test(
  NAME merge-test-with-pipes
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/with-pipes.sh"
)

add_test(
  NAME merge-test-with-sorted-input
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/sorted-input.sh"
)