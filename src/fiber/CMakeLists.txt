file(GLOB_RECURSE SOURCES *.cpp)
add_library(fiber STATIC ${SOURCES})
target_link_libraries(fiber)
