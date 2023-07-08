include(FetchContent)

FetchContent_Declare(Protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG        v3.21.12
        FIND_PACKAGE_ARGS NAMES Protobuf
        )
FetchContent_MakeAvailable(Protobuf)
INCLUDE(FindProtobuf)
FIND_PACKAGE(Protobuf CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER message.proto)
ADD_LIBRARY(proto ${PROTO_HEADER} ${PROTO_SRC})
