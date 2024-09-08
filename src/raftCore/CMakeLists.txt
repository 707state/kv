file(GLOB_RECURSE SOURCES *.cc)
add_library(raftCore STATIC ${SOURCES})
target_link_libraries(raftCore PRIVATE rpc common)
