# FetchContent_Declare(Protobuf GIT_REPOSITORY
# https://github.com/protocolbuffers/protobuf.git GIT_TAG        v3.21.12
# FIND_PACKAGE_ARGS NAMES Protobuf ) FetchContent_MakeAvailable(Protobuf)
include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
protobuf_generate_cpp(PROTO_SRC PROTO_HEADER message.proto)
add_library(proto ${PROTO_HEADER} ${PROTO_SRC} ${Protobuf_LIBRARIES})
