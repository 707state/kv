file(GLOB_RECURSE SOURCES *.cc)
add_library(raftRpcPro STATIC ${SOURCES})
target_link_libraries(raftRpcPro protobuf)
