#include "Connection.h"
#include "proto/message.pb.h"
#include <array>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <vector>


Connection::Connection(ConnectionArgs args) : args(args) {
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
    rv = this->doLogin(args.username, args.password);
    if (rv == -1) {
        perror("Login failed");
        currConnState = ConnectionState::CONNECTION_FAILURE;
    }

}

Connection::~Connection() {
    if (currConnState == ConnectionState::CONNECTED) {
        close(connectionFD);
    }
}

Connection::ConnectionState Connection::getConnectionState() {
    return currConnState;
}

std::optional<std::pair<Header, std::string>> Connection::receiveMsg() {
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


bool Connection::receiveBytes(char *bytes, size_t bytes_to_receive) {
    int received = 0;
    int now = 0;
    while (received < bytes_to_receive) {
        now = recv(connectionFD, bytes, bytes_to_receive, 0);
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

int Connection::doLogin(std::string username, std::string password) {
    sesstionId = -1;
    if (currConnState != ConnectionState::CONNECTED) {
        return -1;
    }
    Request req;
    req.New();
    req.set_type(RequestType::LOGIN);
    req.set_username(username);
    req.set_password(password);

    Message m(req);
    Connection loginConn (*this);
    if (loginConn.getConnectionState() != ConnectionState::CONNECTED) {
        perror("Login failed");
        return -1;
    }
    loginConn.sendMessage(m);

    auto msg = loginConn.receiveMsg();
    if (!msg.has_value()) {
        return -1;
    }
    else {
        auto [header, response] = msg.value();
        Response resp;
        resp.ParseFromArray(response.data(), response.size());

        if (resp.type() == ResponseType::OK) {
            // Extract token
            sesstionId = resp.session_id();
            return 0;
        }
        else {
            return -1;
        }
    }


}

bool Connection::sendMessage(Message msg) {
    if (currConnState != ConnectionState::CONNECTED) {
        return false;
    }
    SealedMessage sm(msg, sesstionId);
    void *buffer = sm.getSealedMessagePtr();
    int hasToSend = sm.getSealedMessageSize();
    int sent = 0;

    while (sent < hasToSend) {
        int sent_now = send(connectionFD, buffer, sm.getSealedMessageSize(), 0);

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

Connection::Connection(Connection &other) : args(other.args) {
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

    sesstionId = other.sesstionId;

}

bool Connection::isLogged() {
    return sesstionId != -1;
}

bool Connection::sendRequest(Request request) {
    Message m(request);
    return sendMessage(m);
}

std::optional<std::pair<Header, Response>> Connection::receiveResponse() {
auto msg = receiveMsg();
    if (!msg.has_value()) {
        return {};
    }
    else {
        auto [header, response] = msg.value();
        Response resp;
        resp.ParseFromString(response);
        return {{header, resp}};
    }
}
