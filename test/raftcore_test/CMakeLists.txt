set(SRC_LIST raftKvDB.cpp)

add_executable(raftCoreRun ${SRC_LIST})
target_link_libraries(
  raftCoreRun
  rpc
  protobuf
  pthread
  raftCore
  raftRpcPro
  raftClerk
  fiber
  common
  absl::log_internal_check_op)

# # # # # # # # # # # # # # # #
# ###############################################################

set(SRC_LIST2 caller.cpp)
add_executable(callerMain ${src_raftClerk} ${SRC_LIST2} ${src_common})
target_link_libraries(
  callerMain
  protobuf
  Boost::serialization
  raftCore
  rpc
  fiber
  raftRpcPro
  raftClerk
  common
  absl::log_internal_check_op)
