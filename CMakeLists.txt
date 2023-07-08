cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)

project(fileSyncerCommon)
add_library(fileSyncerCommon src/common/MessageComunication.cpp)

FetchContent_Declare(Protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG        v3.21.12
        FIND_PACKAGE_ARGS NAMES Protobuf
        )
FetchContent_MakeAvailable(Protobuf)

add_subdirectory(proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})
#add_custom_command(
#        OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
#        COMMAND ${PROTOC} --proto_path ${PROTO_DIR} message.proto --cpp_out ${GENERATED_CODE_DIR}
#        DEPENDS ${PROTOC} ${PROTO_DIR}/message.proto
#)


project(fileSyncerServer)
add_executable(fileSyncerServer src/server/main.cpp ${PROTO_SRCS} ${PROTO_HDRS} src/server/RequestHandler.cpp src/server/RequestHandler.h)
target_link_libraries(fileSyncerServer fileSyncerCommon proto protobuf::libprotobuf protobuf::libprotoc)


project(fileSyncerClient)
add_executable(fileSyncerClient src/client/mainClient.cpp src/client/Connection.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(fileSyncerClient fileSyncerCommon proto protobuf::libprotobuf protobuf::libprotoc)
