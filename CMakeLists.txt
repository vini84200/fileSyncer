cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

project(fileSyncerCommon)
add_library(fileSyncerCommon src/common/MessageComunication.cpp)


project(fileSyncerServer)


add_executable(fileSyncerServer src/server/main.cpp)
target_link_libraries(fileSyncerServer fileSyncerCommon)

project(fileSyncerClient)
add_executable(fileSyncerClient src/client/mainClient.cpp)
target_link_libraries(fileSyncerClient fileSyncerCommon)