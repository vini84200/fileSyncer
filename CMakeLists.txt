cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

project(fileSyncerCommon)
project(fileSyncerServer)
project(fileSyncerClient)

add_library(fileSyncerCommon STATIC src/common/MessageComunication.cpp
        src/common/utils.cpp
        src/common/RwLock.h)
include_directories(${OPENSSL_INCLUDE_DIR})

# FetchContent_Declare(Protobuf GIT_REPOSITORY
# https://github.com/protocolbuffers/protobuf.git GIT_TAG        v3.21.12
# FIND_PACKAGE_ARGS NAMES Protobuf ) FetchContent_MakeAvailable(Protobuf)

add_subdirectory(proto)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})
# add_custom_command( OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} COMMAND ${PROTOC}
# --proto_path ${PROTO_DIR} message.proto --cpp_out ${GENERATED_CODE_DIR}
# DEPENDS ${PROTOC} ${PROTO_DIR}/message.proto )

add_executable(
        fileSyncerServer src/server/main.cpp ${PROTO_SRCS} ${PROTO_HDRS}
        src/server/service/ServerRequestHandler.cpp
        src/server/interfaces/RequestHandler.cpp
        src/server/Server.cpp
        src/server/ServerState.cpp
        src/server/transactions/Transaction.cpp
        src/server/interfaces/Listener.cpp
        src/server/admin/AdminListener.cpp
        src/server/admin/AdminRequestHandler.cpp
        src/server/transactions/TransactionListener.cpp
        src/server/transactions/TransactionListener.h
        src/server/service/ServiceRequestHandler.cpp
        src/server/service/ServiceRequestHandler.h
        src/server/service/ServiceListener.cpp
        src/server/service/ServiceListener.h
        src/server/TransactionManager.cpp
        src/server/TransactionManager.h
        src/server/transactions/CreateUserTransaction.cpp
        src/server/transactions/CreateSessionTransaction.cpp
        src/server/transactions/TransactionRequestHandler.cpp
        src/server/transactions/TransactionRequestHandler.h

)
target_link_libraries(fileSyncerServer fileSyncerCommon proto protobuf
        ${Protobuf_LIBRARIES})

target_link_libraries(
        fileSyncerCommon
        ${OPENSSL_LIBRARIES}
        proto
        protobuf
        ${Protobuf_LIBRARIES}
        ssl
        crypto)

add_executable(
        fileSyncerClient src/client/mainClient.cpp src/client/Connection.cpp
        ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(
        fileSyncerClient
        fileSyncerCommon
        proto
        protobuf
        ${Protobuf_LIBRARIES}
        ssl
        crypto)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++17")
find_package(OpenSSL REQUIRED)
message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
