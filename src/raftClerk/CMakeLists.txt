file(GLOB_RECURSE SOURCES *.cpp)
add_library(raftClerk STATIC ${SOURCES})
target_link_libraries(raftClerk)
