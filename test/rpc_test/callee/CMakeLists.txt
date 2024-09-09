set(SRC_LIST friendService.cpp ../friend.pb.cc)

add_executable(callee ${SRC_LIST})
target_link_libraries(callee rpc pthread protobuf c++abi
                      absl::log_internal_check_op)
