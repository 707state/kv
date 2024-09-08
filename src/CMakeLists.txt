include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${Boost_INCLUDE_DIRS})

link_directories(${BOOST_LIBRARY_DIRS})

add_subdirectory(common)
add_subdirectory(fiber)
add_subdirectory(rpc)
add_subdirectory(raftClerk)
add_subdirectory(raftCore)
add_subdirectory(raftRpcPro)
