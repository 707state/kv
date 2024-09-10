set(SRC_LIST friendService.cpp ../friend.pb.cc)

find_package(absl REQUIRED)
add_executable(callee ${SRC_LIST})
target_link_libraries(
  callee
  rpc
  pthread
  protobuf
  c++abi
  absl::log_internal_check_op
  absl::log_internal_message
  absl::log_internal_proto)
