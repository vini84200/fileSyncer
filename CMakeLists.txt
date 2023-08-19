cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)

project(fileSyncerCommon)
project(fileSyncerServer)
project(fileSyncerClient)

add_library(fileSyncerCommon STATIC
        src/common/utils.cpp
        src/common/RwLock.h
)

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
        fileSyncerServer
        src/server/main.cpp ${PROTO_SRCS} ${PROTO_HDRS}
        src/server/service/ServerRequestHandler.cpp
        src/server/Server.cpp
        src/server/ServerState.cpp
        src/server/transactions/Transaction.cpp
        src/server/admin/AdminListener.cpp
        src/server/admin/AdminRequestHandler.cpp
        src/server/transactions/TransactionListener.cpp
        src/server/transactions/TransactionListener.h
        src/server/service/ServiceRequestHandler.cpp
        src/server/service/ServiceRequestHandler.h
        src/server/service/ServiceListener.cpp
        src/server/service/ServiceListener.h
        src/server/transactions/TransactionManager.cpp
        src/server/transactions/CreateUserTransaction.cpp
        src/server/transactions/CreateSessionTransaction.cpp
        src/server/transactions/TransactionRequestHandler.cpp
        src/server/Replica.cpp
        src/server/transactions/RemoveSessionTransaction.cpp
        src/server/Election.cpp

)
target_link_libraries(fileSyncerServer fileSyncerCommon proto protobuf
        ${Protobuf_LIBRARIES})

target_link_libraries(
        fileSyncerCommon
        proto
        protobuf
        ${Protobuf_LIBRARIES}
        )

add_executable(
        fileSyncerClient
        src/client/mainClient.cpp
        src/client/ClientConnection.cpp
        ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(
        fileSyncerClient
        fileSyncerCommon
        proto
        protobuf
        ${Protobuf_LIBRARIES}
        )

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++17")

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

FetchContent_Declare(
        xxhash_cpp
        GIT_REPOSITORY https://github.com/RedSpah/xxhash_cpp.git
        GIT_TAG        0.8.1
)

FetchContent_MakeAvailable(xxhash_cpp)

target_link_libraries(fileSyncerCommon xxhash_cpp)

FetchContent_Declare(Catch2                              # name of the content
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git  # the repository
        GIT_TAG        v2.13.7                                 # the tag
)

# make available
FetchContent_MakeAvailable(Catch2)

# testing binary
add_executable(cpp_test test/client.cpp)
target_link_libraries(cpp_test PRIVATE Catch2::Catch2 fileSyncerCommon proto protobuf ${Protobuf_LIBRARIES})

# enable testing functionality
enable_testing()

# define tests
add_test(
        NAME catch_test
        COMMAND $<TARGET_FILE:cpp_test> --success
)