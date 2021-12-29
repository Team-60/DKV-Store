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

# Proto filee
get_filename_component(volume_server_proto "./protos/volume_server.proto" ABSOLUTE)
get_filename_component(volume_server_proto_path "${volume_server_proto}" PATH)

# Generated sources
set(volume_server_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/volume_server.pb.cc")
set(volume_server_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/volume_server.pb.h")
set(volume_server_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/volume_server.grpc.pb.cc")
set(volume_server_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/volume_server.grpc.pb.h")

add_custom_command(
  OUTPUT "${volume_server_proto_srcs}" "${volume_server_proto_hdrs}" "${volume_server_grpc_srcs}" "${volume_server_grpc_hdrs}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
    --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
    -I "${volume_server_proto_path}"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    "${volume_server_proto}"
  DEPENDS "${volume_server_proto}")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

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

# add executable server.cc
add_executable(volume_server "server/volume_server/main.cc"
  ${volume_server_proto_srcs}
  ${volume_server_grpc_srcs})
target_link_libraries(volume_server
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})
# add libraries and directores for leveldb
<<<<<<< HEAD
target_link_libraries(master 
  ${LEVELDB_PREFIX}/build/libleveldb.a)
target_link_libraries(master 
  ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(master PUBLIC 
  ${LEVELDB_PREFIX}/include/)
=======
target_link_libraries(volume_server 
  ${CMAKE_SOURCE_DIR}/leveldb/build/libleveldb.a)
target_link_libraries(volume_server 
  ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(volume_server PUBLIC 
  ${CMAKE_SOURCE_DIR}/leveldb/include/)
>>>>>>> 3d2809ff0ed9441f714b1cf919cceca3441e1b49

add_executable(client "client/client.cc"
  ${volume_server_proto_srcs}
  ${volume_server_grpc_srcs})
target_link_libraries(client
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})
