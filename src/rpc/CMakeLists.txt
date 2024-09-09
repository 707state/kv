file(GLOB_RECURSE SOURCES *.cpp)
add_library(rpc STATIC ${SOURCES})
target_link_libraries(rpc protobuf common)
