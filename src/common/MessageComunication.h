#ifndef FILESYNCERCLIENT_MESSAGECOMUNICATION_H
#define FILESYNCERCLIENT_MESSAGECOMUNICATION_H

#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include "proto/message.pb.h"

struct Header {
    char header[18];
    long int msg_size;
    long int session_id;

    Header() {
        strcpy(header, "VINIPROTOCOL v0.1");
        msg_size = 0;
        session_id = 0;
    }

    Header(long int msg_size, long int session_id) {
        strcpy(header, "VINIPROTOCOL v0.1");
        this->msg_size = msg_size;
        this->session_id = session_id;
    }

    static constexpr int getHeaderSize() {
        return sizeof(header) + sizeof(msg_size) + sizeof(session_id);
    }

    void writeIntoBuffer(char *buffer) {
        if (buffer == nullptr) return;
        memcpy(buffer, header, sizeof(header));
        memcpy(buffer + sizeof(header), &msg_size, sizeof(msg_size));
        memcpy(buffer + sizeof(header) + sizeof(msg_size), &session_id, sizeof(session_id));
    }

    int loadFromBuffer(char *buffer) {
        if (buffer == nullptr) return -1;

        if (memcmp(buffer, header, sizeof(header)) != 0) return -1;

        memcpy(&msg_size, buffer + sizeof(header), sizeof(msg_size));
        memcpy(&session_id, buffer + sizeof(header) + sizeof(msg_size), sizeof(session_id));
        return 0;
    }

    int getBufferOffset() {
        return getHeaderSize();
    }

};


class Message {
    std::string buffer;

public:
    Message() {
    }

    Message(char *buffer, int size) {
        this->buffer.assign(buffer, buffer + size);
    }


    int getSize() {
        return buffer.size();
    }

    std::string getVector() {
        return buffer;
    }

    char *getBuffer() {
        return buffer.data();
    }

    Message(Request request);

    Message(Response response);
};

class SealedMessage {
    Message message;
    Header header;
public:

    SealedMessage() {
    }

    SealedMessage(Message m) {
        message = m;
        header = Header(m.getSize(), 0);
    }

    SealedMessage(Message m, long int session_id) {
        message = m;
        header = Header(m.getSize(), session_id);
    }

    static SealedMessage getFromBuffer(char *buffer) {
        SealedMessage m;
        m.header.loadFromBuffer(buffer);
        m.message = Message(buffer + m.header.getHeaderSize(), m.header.msg_size);
        return m;
    }

    void *getSealedMessagePtr() {
        char *buffer = new char[getSealedMessageSize()];
        header.writeIntoBuffer(buffer);
        memcpy(buffer + header.getHeaderSize(), message.getBuffer(), message.getSize());
        return buffer;
    }

    size_t getSealedMessageSize() {
        return header.getHeaderSize() + message.getSize();
    }

    Message getMessage() {
        return message;
    }
};

#endif //FILESYNCERCLIENT_MESSAGECOMUNICATION_H
