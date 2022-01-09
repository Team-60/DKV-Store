# Copyright 2021 the gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ keyvaluestore example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building keyvaluestore.

cmake_minimum_required(VERSION 3.5.1)

project(DKVStore CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# find thread library  
find_package(Threads REQUIRED)

# Assumes that gRPC and all its dependencies are already installed
# on this system, so they can be located by find_package().

# Find Protobuf installation
# Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file - volume_server
get_filename_component(volume_server_proto "./protos/volume_server.proto" ABSOLUTE)
get_filename_component(volume_server_proto_path "${volume_server_proto}" PATH)
# Proto file - shard_master
get_filename_component(shard_master_proto "./protos/shard_master.proto" ABSOLUTE)
get_filename_component(shard_master_proto_path "${shard_master_proto}" PATH)

# Generated sources - volume_server
set(volume_server_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/volume_server.pb.cc")
set(volume_server_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/volume_server.pb.h")
set(volume_server_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/volume_server.grpc.pb.cc")
set(volume_server_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/volume_server.grpc.pb.h")
# Generated sources - shard_master
set(shard_master_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/shard_master.pb.cc")
set(shard_master_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/shard_master.pb.h")
set(shard_master_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/shard_master.grpc.pb.cc")
set(shard_master_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/shard_master.grpc.pb.h")

add_custom_command(
  OUTPUT "${volume_server_proto_srcs}" "${volume_server_proto_hdrs}" "${volume_server_grpc_srcs}" "${volume_server_grpc_hdrs}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
    --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
    -I "${volume_server_proto_path}"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    "${volume_server_proto}"
  DEPENDS "${volume_server_proto}")

add_custom_command(
  OUTPUT "${shard_master_proto_srcs}" "${shard_master_proto_hdrs}" "${shard_master_grpc_srcs}" "${shard_master_grpc_hdrs}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
    --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
    -I "${shard_master_proto_path}"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    "${shard_master_proto}"
  DEPENDS "${shard_master_proto}")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# utils
set("utils_srcs" "${CMAKE_SOURCE_DIR}/utils")
set("shard_master_srcs" "${CMAKE_SOURCE_DIR}/server/shard_master")
set("volume_server_srcs" "${CMAKE_SOURCE_DIR}/server/volume_server")
#set("thread_pool" "${CMAKE_SOURCE_DIR}/utils/thread-pool-2.0.0")

# volume_server_grpc_proto
add_library(volume_server_grpc_proto
  ${volume_server_grpc_srcs}
  ${volume_server_grpc_hdrs}
  ${volume_server_proto_srcs}
  ${volume_server_proto_hdrs})
target_link_libraries(volume_server_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# shard_master_grpc_proto
add_library(shard_master_grpc_proto
  ${shard_master_grpc_srcs}
  ${shard_master_grpc_hdrs}
  ${shard_master_proto_srcs}
  ${shard_master_proto_hdrs})
target_link_libraries(shard_master_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# add executable shard_master.cc
add_executable(shard_master "server/shard_master/main.cc")
target_link_libraries(shard_master
  shard_master_grpc_proto
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  ${LEVELDB_PREFIX}/build/libleveldb.a
  ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(shard_master PUBLIC 
  ${utils_srcs}
  ${LEVELDB_PREFIX}/include/)

# add executable volume_server.cc
add_executable(volume_server "server/volume_server/main.cc" ${utils_srcs}/md5.cc)
target_link_libraries(volume_server
  shard_master_grpc_proto
  volume_server_grpc_proto
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  ${LEVELDB_PREFIX}/build/libleveldb.a
  ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(volume_server PUBLIC 
  ${utils_srcs} 
  ${thread_pool}
  ${LEVELDB_PREFIX}/include/)

# client executable
add_executable(client "client/client.cc" ${utils_srcs}/md5.cc)
target_link_libraries(client
  shard_master_grpc_proto
  volume_server_grpc_proto
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})
target_include_directories(client PUBLIC ${utils_srcs})

# tests
if(BUILD_TESTS)
  file(GLOB TEST_SOURCES tests/*/*.cc)
  foreach(testsourcefile ${TEST_SOURCES})
    get_filename_component(testname ${testsourcefile} NAME_WE)
    add_executable(${testname} ${testsourcefile}
      ${shard_master_proto_srcs}
      ${shard_master_grpc_srcs}
      ${volume_server_proto_srcs}
      ${volume_server_grpc_srcs}
      ${utils_srcs}/md5.cc
      "server/shard_master/shard_master.cc"
      "server/volume_server/volume_server.cc"
      test_utils/test_utils.cc test_utils/test_utils.h)
    target_link_libraries(${testname}
      ${_GRPC_GRPCPP}
      ${_PROTOBUF_LIBPROTOBUF}
      ${LEVELDB_PREFIX}/build/libleveldb.a
      ${CMAKE_THREAD_LIBS_INIT})
    target_include_directories(${testname} PUBLIC 
      ${LEVELDB_PREFIX}/include/
      ${utils_srcs}
      ${shard_master_srcs}
      ${volume_server_srcs})
  endforeach(testsourcefile ${TEST_SOURCES})
  add_custom_target(check ./test.sh)
endif(BUILD_TESTS)
