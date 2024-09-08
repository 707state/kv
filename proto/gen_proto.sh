#!/usr/bin/sh
protoc --cpp_out=../src/raftRpcPro/ raftRPC.proto && mv ../src/raftRpcPro/raftRPC.pb.h ../include/raftRpcPro/
protoc --cpp_out=../src/raftRpcPro/ kvServerRPC.proto && mv ../src/raftRpcPro/kvServerRPC.pb.h ../include/raftRpcPro/
protoc --cpp_out=../src/rpc/ rpcheader.proto && mv ../src/rpc/rpcheader.pb.h ../include/rpc/
