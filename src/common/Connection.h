#ifndef FILESYNCER_CONNECTION_H
#define FILESYNCER_CONNECTION_H

#include "proto/message.pb.h"
#include <cstddef>

#define CONNECTION_WAS_CLOSED 0
#define SEND_ERROR -1


#include "../common/MessageComunication.h"
#include <csignal>
#include <optional>
#include <string>
#include <netdb.h>

struct ConnectionArgs {
    std::string hostname;
    int port;

    std::string username;
    std::string password;

    ConnectionArgs(
            std::string hostname,
            int port,
            std::string username,
            std::string password
    ) : hostname(hostname), port(port), username(username), password(password) {};

    ConnectionArgs(
            std::string hostname,
            int port
    ) : hostname(hostname), port(port) {};
};

enum class ConnectionState {
    NOT_INITIALIZED,
    CONNECTED,
    CONNECTION_CLOSED,
    CONNECTION_FAILURE
};

template<typename Req, typename Res>
class Connection {

public:
    Connection(ConnectionArgs args);

    Connection(const Connection &other); // copy constructor
    // move constructor
    Connection(Connection &&other) noexcept;
    ~Connection();


    ConnectionState getConnectionState();

    void setTimout(int ms) {
        struct timeval tv;
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
        setsockopt(connectionFD, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof tv);
    }

public:
    std::optional<std::pair<Header, std::string>> receiveMsg();

    std::optional<std::pair<Header, Res>> receiveResponse();

    bool receiveBytes(char *bytes, size_t bytes_to_receive);

    bool sendMessage(Message msg);

    bool sendRequest(Req request);

    virtual int getSessionId() {
        return 0;
    }


private:
    int connectionFD = 0;
    ConnectionArgs args;

protected:
    ConnectionState currConnState = ConnectionState::NOT_INITIALIZED;
};

template<typename Req, typename Res>
Connection<Req, Res>::Connection(ConnectionArgs args) : args(args) {
    currConnState = ConnectionState::NOT_INITIALIZED;
    struct addrinfo hints, *servinfo, *p;
    int rv;


    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(args.hostname.c_str(), std::to_string(args.port).c_str(), &hints, &servinfo)) != 0) {
        currConnState = ConnectionState::CONNECTION_FAILURE;
        return;
    }

    for (p = servinfo; p != nullptr; p = p->ai_next) {
        if ((connectionFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            currConnState = ConnectionState::CONNECTION_FAILURE;
            continue;
        }

        if (connect(connectionFD, p->ai_addr, p->ai_addrlen) == -1) {
            currConnState = ConnectionState::CONNECTION_FAILURE;
            close(connectionFD);
            continue;
        }
        break;
    }

    if (p == NULL) {
        currConnState = ConnectionState::CONNECTION_FAILURE;
    } else {
        currConnState = ConnectionState::CONNECTED;
    }

    freeaddrinfo(servinfo);

    if (args.username.empty() || args.password.empty()) {
        return;
    }

}

template<typename Req, typename Res>
Connection<Req, Res>::~Connection() {
    if (currConnState == ConnectionState::CONNECTED) {
        close(connectionFD);
    }
}

template<typename Req, typename Res>
ConnectionState Connection<Req, Res>::getConnectionState() {
    return currConnState;
}

template<typename Req, typename Res>
std::optional<std::pair<Header, std::string>> Connection<Req, Res>::receiveMsg() {
    std::array<char, Header::getHeaderSize()> header_buff{};
    bool ok = receiveBytes(header_buff.data(), Header::getHeaderSize());
    if (!ok) {
        return {};
    }

    Header h;
    if (h.loadFromBuffer(header_buff.data()) == -1) {
        return {};
    }

    std::string msg_buff;
    msg_buff.assign(h.msg_size, 0);

    if (!receiveBytes(msg_buff.data(), h.msg_size)) {
        return {};
    }

    return {{h, msg_buff}};
}


template<typename Req, typename Res>
bool Connection<Req, Res>::receiveBytes(char *bytes, size_t bytes_to_receive) {
    size_t received = 0;
    size_t now = 0;
    while (received < bytes_to_receive) {
        now = recv(connectionFD, bytes + received, bytes_to_receive, 0);
        if (now == CONNECTION_WAS_CLOSED) {
            currConnState = ConnectionState::CONNECTION_CLOSED;
            return false;
        }
        if (now == SEND_ERROR) {
            currConnState = ConnectionState::CONNECTION_FAILURE;
            return false;
        }
        received += now;
    }
    return true;
}

template<typename Req, typename Res>
bool Connection<Req, Res>::sendMessage(Message msg) {
    if (currConnState != ConnectionState::CONNECTED) {
        return false;
    }
    SealedMessage sm(msg, getSessionId());
    char *buffer = (char*) sm.getSealedMessagePtr();
    size_t hasToSend = sm.getSealedMessageSize();
    size_t sent = 0;

    while (sent < hasToSend) {
        int sent_now = send(connectionFD, buffer + sent, sm.getSealedMessageSize() - sent, 0);

        if (sent_now == CONNECTION_WAS_CLOSED) {
            currConnState = ConnectionState::CONNECTION_CLOSED;
            return false;
        }
        if (sent_now == SEND_ERROR) {
            currConnState = ConnectionState::CONNECTION_FAILURE;
            return false;
        }
        sent += sent_now;
    }

    return true;
}

template<typename Req, typename Res>
Connection<Req, Res>::Connection(const Connection &other) : args(other.args) {
    currConnState = ConnectionState::NOT_INITIALIZED;
    struct addrinfo hints, *servinfo, *p;
    int rv;


    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(args.hostname.c_str(), std::to_string(args.port).c_str(), &hints, &servinfo)) != 0) {
        currConnState = ConnectionState::CONNECTION_FAILURE;
        return;
    }

    for (p = servinfo; p != nullptr; p = p->ai_next) {
        if ((connectionFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            currConnState = ConnectionState::CONNECTION_FAILURE;
            continue;
        }

        if (connect(connectionFD, p->ai_addr, p->ai_addrlen) == -1) {
            currConnState = ConnectionState::CONNECTION_FAILURE;
            close(connectionFD);
            continue;
        }
        break;
    }

    if (p == NULL) {
        currConnState = ConnectionState::CONNECTION_FAILURE;
    } else {
        currConnState = ConnectionState::CONNECTED;
    }

    freeaddrinfo(servinfo);
}

template<typename Req, typename Res>
Connection<Req, Res>::Connection(Connection<Req, Res> &&other)  noexcept = default;

template<typename Req, typename Res>
bool Connection<Req, Res>::sendRequest(Req request) {
    Message m(request);
    return sendMessage(m);
}
template<typename Req, typename Res>
std::optional<std::pair<Header, Res>> Connection<Req, Res>::receiveResponse() {
    auto msg = receiveMsg();
    if (!msg.has_value()) {
        return {};
    } else {
        auto [header, response] = msg.value();
        Res resp;
        resp.ParseFromString(response);
        return {{header, resp}};
    }
}

#endif
