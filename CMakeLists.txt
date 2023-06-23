cmake_minimum_required(VERSION 3.25)
project(fileSyncerServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(fileSyncer main.cpp MessageComunication.cpp MessageComunication.h)

project(fileSyncerClient)

add_executable(fileSyncerClient mainClient.cpp)