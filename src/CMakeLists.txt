set(MERGERESULTS_SRC
    cache/InternedString.cpp
    cache/Aggregate.cpp
    cache/MeasureFile.cpp
    cache/ResultSet.cpp
    MergeResult.cpp
)

add_executable(mergeresults ${MERGERESULTS_SRC})
add_dependencies(mergeresults rapidjson)
target_link_libraries(mergeresults)

set(AGGREGATOR_SRC
    cache/InternedString.cpp
    cache/Aggregate.cpp
    cache/MeasureFile.cpp
    cache/ResultSet.cpp
    Aggregator.cpp
)

include_directories(${LIBLZMA_INCLUDE_DIRS})
add_executable(aggregator ${AGGREGATOR_SRC})
add_dependencies(aggregator rapidjson)
target_link_libraries(aggregator ${LIBLZMA_LIBRARIES})
