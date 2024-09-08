file(GLOB_RECURSE SOURCES *.cpp)
add_library(common STATIC ${SOURCES})
target_link_libraries(common PRIVATE Boost::serialization)
