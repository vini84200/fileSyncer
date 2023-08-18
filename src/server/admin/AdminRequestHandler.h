//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_ADMINREQUESTHANDLER_H
#define FILESYNCERCLIENT_ADMINREQUESTHANDLER_H

#include "../interfaces/RequestHandler.h"

class Server;

class AdminRequestHandler : public RequestHandler<AdminMsg> {
private:
    Server *server;
public:
    AdminRequestHandler(int socket, Server *server) : RequestHandler(socket), server(server) {}
    void handleRequest() override;
    void handleHeartbeat();
};


#endif//FILESYNCERCLIENT_ADMINREQUESTHANDLER_H
