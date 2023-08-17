//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_REQUESTHANDLER_H
#define FILESYNCERCLIENT_REQUESTHANDLER_H

#include "../../common/MessageComunication.h"
#include "proto/message.pb.h"
#include <optional>
#include <string>
#include <sys/socket.h>

class RequestHandler {
protected:
    int client_fd;
    pthread_t thread_;

    pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

    bool receiveBytes(char *bytes, size_t bytes_to_receive);

public:
    RequestHandler(int socket);
    std::optional<std::pair<Header, std::string>> receiveMsg();
    virtual void handleRequest() = 0;
    bool sendMessage(Message msg);
    pthread_t start();
    RequestHandler(sockaddr_storage client_addr, int client_fd);
    bool endConnection();
    std::optional<std::pair<Header, Request>> receiveRequest();
    void stop();
    bool is_running = true;
};

#endif// FILESYNCERCLIENT_REQUESTHANDLER_H
