//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_REQUESTHANDLER_H
#define FILESYNCERCLIENT_REQUESTHANDLER_H

#include "../../common/Connection.h"
#include "../../common/MessageComunication.h"
#include "proto/message.pb.h"
#include <csignal>
#include <optional>
#include <string>
#include <sys/socket.h>

template <typename T>
class RequestHandler {
protected:
    int client_fd;
    bool is_running = true;
    pthread_t thread_;

    pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

    bool receiveBytes(char *bytes, size_t bytes_to_receive);
    std::optional<std::pair<Header, std::string>> receiveMsg();
public:
    RequestHandler(int socket);
    RequestHandler(sockaddr_storage client_addr, int client_fd);

    virtual void handleRequest() = 0;
    std::optional<std::pair<Header, T>> receiveRequest();

    bool sendMessage(Message msg);
    pthread_t start();
    bool endConnection();
    void stop();
};

template<typename T>
std::optional<std::pair<Header, std::string>>
RequestHandler<T>::receiveMsg() {
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

template<typename T>
bool RequestHandler<T>::receiveBytes(char *bytes,
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

template<typename T>
bool RequestHandler<T>::sendMessage(const Message msg) {
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

template<typename T>
void *run(void *args) {
    auto *rh = (RequestHandler<T> *) args;
    rh->handleRequest();
    return nullptr;
}

template<typename T>
pthread_t RequestHandler<T>::start() {
    if (pthread_create(&thread_, nullptr, run<T>, this) != 0) {
        perror("Error creating thread");
        exit(1);
    }
}

template<typename T>
RequestHandler<T>::RequestHandler(sockaddr_storage client_addr,
                                  int client_fd)
        : client_fd(client_fd) {
}

template<typename T>
bool RequestHandler<T>::endConnection() {
    close(client_fd);
}

template<typename T>
std::optional<std::pair<Header, T>>
RequestHandler<T>::receiveRequest() {
    auto msg = receiveMsg();
    if (!msg.has_value()) { return {}; }

    auto h = msg.value().first;
    auto m = msg.value().second;

    T r;
    if (r.ParseFromString(m) == 0) { return {}; }

    return {{h, r}};
}

template<typename T>
void RequestHandler<T>::stop() {
    // Close connection
    is_running = false;
    endConnection();
    pthread_join(thread_, nullptr);
}

template<typename T>
RequestHandler<T>::RequestHandler(int socket) {
    client_fd = socket;
}
#endif// FILESYNCERCLIENT_REQUESTHANDLER_H
