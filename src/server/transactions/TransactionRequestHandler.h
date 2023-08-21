//
// Created by vini84200 on 8/17/23.
//

#ifndef FILESYNCERCLIENT_TRANSACTIONREQUESTHANDLER_H
#define FILESYNCERCLIENT_TRANSACTIONREQUESTHANDLER_H

#include "../interfaces/RequestHandler.h"

class Server;
class TransactionRequestHandler : public RequestHandler<TransactionOuterMsg> {
    void handleRequest() override;
private:
    Server* server;

public:
    TransactionRequestHandler(int socket, Server *server) : RequestHandler(socket), server(server) {}
};


#endif//FILESYNCERCLIENT_TRANSACTIONREQUESTHANDLER_H
