set(Boost_USE_STATIC_LIBS ON)

find_package(absl REQUIRED)
add_subdirectory(fiber_test)
add_subdirectory(rpc_test)
add_subdirectory(./raftcore_test/)
add_subdirectory(asio_test)
