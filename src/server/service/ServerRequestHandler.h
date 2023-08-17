//
// Created by vini84200 on 7/7/23.
//

#ifndef FILESYNCERCLIENT_SERVERREQUESTHANDLER_H
#define FILESYNCERCLIENT_SERVERREQUESTHANDLER_H

#include "../interfaces/RequestHandler.h"
#include "proto/message.pb.h"
#include <optional>
#include <string>
#include <sys/socket.h>

// This class will handle the requests from the client
class ServerRequestHandler : public RequestHandler<Request> {

public:
    // Will run in a separate thread
    void handleRequest() override;
    bool sendResponse(Response response);

    ServerRequestHandler(sockaddr_storage client_addr, int client_fd)
        : RequestHandler(client_addr, client_fd) {}

private:
    void handleLogin(Request request, Header header);

    void handlePing(Request request, Header header);

    void handleSubscribe(Request request, Header header);

    void fileUpdate(Request request, Header header);

    void list(Request request, Header header);

    void handleDownload(Request request, Header header);
};


#endif// FILESYNCERCLIENT_SERVERREQUESTHANDLER_H
