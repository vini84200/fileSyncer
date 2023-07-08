//
// Created by vini84200 on 7/7/23.
//

#ifndef FILESYNCERCLIENT_REQUESTHANDLER_H
#define FILESYNCERCLIENT_REQUESTHANDLER_H


#include <optional>
#include <string>
#include <sys/socket.h>
#include "../common/MessageComunication.h"
#include "proto/message.pb.h"

// This class will handle the requests from the client
class RequestHandler {

public:
    RequestHandler(sockaddr_storage client_addr, int client_fd);

    pthread_t start();

    // Will run in a separate thread
    void handleRequest();

    std::optional<std::pair<Header, std::string>> receiveMsg();

    bool sendMessage(Message msg);

    bool endConnection();

    std::optional<std::pair<Header, Request>> receiveRequest();

    bool sendResponse(Response response);

private:
    bool receiveBytes(char *bytes, size_t bytes_to_receive);


    sockaddr_storage *client_addr;
    int client_fd;
    pthread_t thread_;

    void handleLogin(Request request, Header header);

    void handlePing(Request request, Header header);

    void handleSubscribe(Request request, Header header);

    void fileUpdate(Request request, Header header);
};


#endif //FILESYNCERCLIENT_REQUESTHANDLER_H
