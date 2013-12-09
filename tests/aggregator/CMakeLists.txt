file(COPY ss-ff-n-22.lzma ss-ff-n-28.lzma input.txt DESTINATION .)

add_test(
  NAME aggregator-test
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/aggregator-test.sh"
)
