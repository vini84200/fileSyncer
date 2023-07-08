//
// Created by vini84200 on 7/7/23.
//

#include "RequestHandler.h"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <utility>
#include <csignal>
#include "../client/Connection.h"
#include "proto/message.pb.h"

std::optional<std::pair<Header, std::string>> RequestHandler::receiveMsg() {
    std::array<char, Header::getHeaderSize()> header_buff{};
    bool ok = receiveBytes(header_buff.data(), Header::getHeaderSize());
    if (!ok) {
        return {};
    }

    Header h;
    if (h.loadFromBuffer(header_buff.data()) == -1) {
        perror("Error loading header");
        endConnection();
        return {};
    }

    std::string msg_buff;
    msg_buff.assign(h.msg_size, 0);

    if (!receiveBytes(msg_buff.data(), h.msg_size)) {
        return {};
    }

    return {{h, msg_buff}};
}

bool RequestHandler::receiveBytes(char *bytes, size_t bytes_to_receive) {
    int received = 0;
    int now = 0;
    while (received < bytes_to_receive) {
        now = recv(client_fd, bytes, bytes_to_receive, 0);
        if (now == CONNECTION_WAS_CLOSED) {
            return false;
        }
        if (now == SEND_ERROR) {
            return false;
        }
        received += now;
    }
    return true;
}

bool RequestHandler::sendMessage(Message msg) {
    SealedMessage sm(std::move(msg), -1);
    void *buffer = sm.getSealedMessagePtr();
    int hasToSend = sm.getSealedMessageSize();
    int sent = 0;

    while (sent < hasToSend) {
        int sent_now = send(client_fd, buffer, sm.getSealedMessageSize(), 0);

        if (sent_now == CONNECTION_WAS_CLOSED) {
            return false;
        }
        if (sent_now == SEND_ERROR) {
            return false;
        }
        sent += sent_now;
    }

    return true;
}

void *run(void *args) {
    auto *rh = (RequestHandler *) args;
    rh->handleRequest();
    return nullptr;
}

pthread_t RequestHandler::start() {
    if (pthread_create(&thread_, nullptr, run, this) != 0) {
        perror("Error creating thread");
        exit(1);
    }
}

RequestHandler::RequestHandler(sockaddr_storage client_addr, int client_fd) :
        client_addr(&client_addr), client_fd(client_fd) {

}

void RequestHandler::handleRequest() {
    printf("Handling request\n");
    auto msg = receiveRequest();
    if (!msg.has_value()) {
        printf("Error receiving request\n");
        endConnection();
        return;
    }
    auto h = msg.value().first;
    auto r = msg.value().second;

    switch (r.type()) {
        case PING:
            printf("Ping request\n");
            break;
        case SUBSCRIBE:
            printf("Subscribe request\n");
            break;
        case DOWNLOAD:
            printf("Download request\n");
            break;
        case UPLOAD:
            printf("Upload request\n");
            break;
        case LIST:
            printf("List request\n");
            break;
        case LOGIN:
            printf("Login request\n");
            handleLogin(r, h);
            break;
        case LOGOUT:
            printf("Logout request\n");
            break;
        default:
            perror("Unknown request type");
            exit(1);
    }

    endConnection();
}

bool RequestHandler::endConnection() {
    close(client_fd);
}

std::optional<std::pair<Header, Request>> RequestHandler::receiveRequest() {
    auto msg = receiveMsg();
    if (!msg.has_value()) {
        return {};
    }

    auto h = msg.value().first;
    auto m = msg.value().second;

    Request r;
    if (r.ParseFromString(m) == 0) {
        return {};
    }

    return {{h, r}};
}

void RequestHandler::handleLogin(Request request, Header header) {
    if (request.type() != LOGIN) {
        perror("Invalid request type");
        exit(1);
    }

    auto login = request.username();
    auto password = request.password();

    // TODO: check login and password
    if (login == "admin" && password == "admin") {
        printf("Login successful\n");
        Response response;
        response.set_type(OK);
        response.set_session_id(1);
        sendResponse(response);
    } else {
        printf("Login failed\n");
        printf("Login: %s\n", login.c_str());
        printf("Password: %s\n", password.c_str());
        Response response;
        response.set_type(ERROR);
        response.set_error_message("Invalid login or password");
        sendResponse(response);
    }
}

bool RequestHandler::sendResponse(Response response) {
    Message msg(std::move(response));
    return sendMessage(msg);
}
