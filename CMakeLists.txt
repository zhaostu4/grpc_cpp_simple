# Copyright 2018 gRPC authors.
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
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

cmake_minimum_required(VERSION 3.19)
project(rpcTest)

set(CMAKE_CXX_STANDARD 14)
set(FLAG_USE_SIMPLE TRUE)
include(./common.cmake)

set(utls_DIR "${CMAKE_SOURCE_DIR}/utls")

# Proto file
# get_filename_component(proto "./protos/simple.proto" ABSOLUTE)
# Generated sources
set(protos_generated_DIR "${CMAKE_SOURCE_DIR}/protosGenerated")

if(${FLAG_USE_SIMPLE})
    get_filename_component(proto "./protos/simple.proto" ABSOLUTE)
    get_filename_component(proto_path "${proto}" PATH)

    set(hw_proto_srcs "${protos_generated_DIR}/simple.pb.cc")
    set(hw_proto_hdrs "${protos_generated_DIR}/simple.pb.h")
    set(hw_grpc_srcs "${protos_generated_DIR}/simple.grpc.pb.cc")
    set(hw_grpc_hdrs "${protos_generated_DIR}/simple.grpc.pb.h")
else()
    get_filename_component(proto "./protos/experiment.proto" ABSOLUTE)
    get_filename_component(proto_path "${proto}" PATH)

    set(hw_proto_srcs "${protos_generated_DIR}/experiment.pb.cc")
    set(hw_proto_hdrs "${protos_generated_DIR}/experiment.pb.h")
    set(hw_grpc_srcs "${protos_generated_DIR}/experiment.grpc.pb.cc")
    set(hw_grpc_hdrs "${protos_generated_DIR}/experiment.grpc.pb.h")
endif()

add_custom_command(
        OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${protos_generated_DIR}"
        --cpp_out "${protos_generated_DIR}"
        -I "${proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${proto}"
        DEPENDS "${proto}")

# Include generated *.pb.h files
include_directories("${protos_generated_DIR}")
# hw_grpc_proto
add_library(hw_grpc_proto
        ${hw_grpc_srcs}
        ${hw_grpc_hdrs}
        ${hw_proto_srcs}
        ${hw_proto_hdrs})
target_link_libraries(hw_grpc_proto
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})

# Thread Pool and randomInt
include_directories("${utls_DIR}")

# Targets greeter_[async_](client|server)
foreach(_target
        # sync_server_zhao_stream sync_client_zhao_stream
        # sync_client sync_server
        # async_server async_service_threadPool async_client
        async_server_zhao async_client_zhao async_client async_service_threadPool
        )
    add_executable(${_target} "${_target}.cpp")
    target_link_libraries(${_target}
            hw_grpc_proto
            ${_REFLECTION}
            ${_GRPC_GRPCPP}
            ${_PROTOBUF_LIBPROTOBUF})
endforeach()
