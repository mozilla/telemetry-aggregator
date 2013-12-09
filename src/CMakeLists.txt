# Common sources
set(COMMON_SRC
    cache/InternedString.cpp
    cache/Aggregate.cpp
    cache/MeasureFile.cpp
    cache/ResultSet.cpp
    cache/ChannelVersion.cpp
    CompressedFileReader.cpp
    ../stringencoders/modp_numtoa.c
)

set(MERGERESULTS_SRC
    ${COMMON_SRC}
    MergeResult.cpp
)

add_executable(../mergeresults ${MERGERESULTS_SRC})
add_dependencies(../mergeresults rapidjson)
target_link_libraries(../mergeresults ${LIBLZMA_LIBRARIES})

set(AGGREGATOR_SRC
    ${COMMON_SRC}
    Aggregator.cpp
)

add_executable(../aggregator ${AGGREGATOR_SRC})
add_dependencies(../aggregator rapidjson)
target_link_libraries(../aggregator ${LIBLZMA_LIBRARIES})

file(COPY processor DESTINATION .)

set(AGGREGATOR_BUNDLE_SRC
    ../aggregator
    processor
)

add_custom_target(
      aggregator_bundle ALL
      COMMAND tar "-czf" "../aggregator_bundle.tar.gz" ${AGGREGATOR_BUNDLE_SRC}
)