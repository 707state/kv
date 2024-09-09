set(SRC_LIST ./callFriendService.cpp ../friend.pb.cc)
add_executable(caller ${SRC_LIST})
target_link_libraries(caller rpc protobuf c++abi absl::log_internal_check_op)
