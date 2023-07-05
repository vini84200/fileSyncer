cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)

project(fileSyncerCommon)
add_library(fileSyncerCommon src/common/MessageComunication.cpp)

FetchContent_Declare(protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG        v21.4
        FIND_PACKAGE_ARGS NAMES protobuf
        )
FetchContent_MakeAvailable(protobuf)

include(FindProtobuf)
find_package(protobuf CONFIG REQUIRED)

set(GENERATED_CODE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(PROTO_SRCS ${GENERATED_CODE_DIR}/message.pb.cc)
set(PROTO_HDRS ${GENERATED_CODE_DIR}/message.pb.h)
set(PROTOC ${protobuf_BINARY_DIR}/protoc)
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(
        OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
        COMMAND ${PROTOC} --proto_path ${PROTO_DIR} message.proto --cpp_out ${GENERATED_CODE_DIR}
        DEPENDS ${PROTOC} ${PROTO_DIR}/message.proto
)


project(fileSyncerServer)
add_executable(fileSyncerServer src/server/main.cpp)
target_link_libraries(fileSyncerServer fileSyncerCommon)

project(fileSyncerClient)
add_executable(fileSyncerClient src/client/mainClient.cpp src/client/Connection.cpp)
target_link_libraries(fileSyncerClient fileSyncerCommon)

