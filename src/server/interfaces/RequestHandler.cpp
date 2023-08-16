//
// Created by vini84200 on 16/08/23.
//

#include "RequestHandler.h"
#include "../../client/Connection.h"
#include "proto/message.pb.h"
#include <bits/types/sigset_t.h>
#include <condition_variable>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <openssl/sha.h>
#include <string>
#include <sys/stat.h>
#include <tuple>
#include <unistd.h>
#include <utility>
#include <vector>

std::optional<std::pair<Header, std::string>>
RequestHandler::receiveMsg() {
    std::array<char, Header::getHeaderSize()> header_buff{};
    bool ok =
            receiveBytes(header_buff.data(), Header::getHeaderSize());
    if (!ok) { return {}; }

    Header h;
    if (h.loadFromBuffer(header_buff.data()) == -1) {
        perror("Error loading header");
        endConnection();
        return {};
    }

    std::string msg_buff;
    msg_buff.assign(h.msg_size, 0);

    if (!receiveBytes(msg_buff.data(), h.msg_size)) { return {}; }

    return {{h, msg_buff}};
}

bool RequestHandler::receiveBytes(char *bytes,
                                  size_t bytes_to_receive) {
    size_t received = 0;
    size_t now      = 0;
    pthread_mutex_lock(&this->mutex_);
    while (received < bytes_to_receive) {
        now = recv(client_fd, bytes + received, bytes_to_receive, 0);
        if (now == CONNECTION_WAS_CLOSED) {
            pthread_mutex_unlock(&this->mutex_);
            return false;
        }
        if (now == SEND_ERROR) {
            pthread_mutex_unlock(&this->mutex_);
            return false;
        }
        received += now;
    }
    pthread_mutex_unlock(&this->mutex_);
    return true;
}

bool RequestHandler::sendMessage(const Message msg) {
    SealedMessage sm(msg, -1);
    char *buffer     = (char *) sm.getSealedMessagePtr();
    size_t hasToSend = sm.getSealedMessageSize();
    size_t sent      = 0;

    pthread_mutex_lock(&this->mutex_);
    while (sent < hasToSend) {
        int sent_now =
                send(client_fd, buffer + sent, hasToSend - sent, 0);

        if (sent_now == CONNECTION_WAS_CLOSED) {
            pthread_mutex_unlock(&this->mutex_);
            return false;
        }
        if (sent_now == SEND_ERROR) {
            pthread_mutex_unlock(&this->mutex_);
            return false;
        }
        sent += sent_now;
    }
    pthread_mutex_unlock(&this->mutex_);

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

RequestHandler::RequestHandler(sockaddr_storage client_addr,
                               int client_fd)
    : client_addr(&client_addr), client_fd(client_fd) {
}

bool RequestHandler::endConnection() {
    close(client_fd);
}

std::optional<std::pair<Header, Request>>
RequestHandler::receiveRequest() {
    auto msg = receiveMsg();
    if (!msg.has_value()) { return {}; }

    auto h = msg.value().first;
    auto m = msg.value().second;

    Request r;
    if (r.ParseFromString(m) == 0) { return {}; }

    return {{h, r}};
}

void RequestHandler::stop() {
    // Close connection
    is_running = false;
    endConnection();
    pthread_join(thread_, nullptr);
}
