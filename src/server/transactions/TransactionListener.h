//
// Created by vini84200 on 8/16/23.
//

#ifndef FILESYNCERCLIENT_TRANSACTIONLISTENER_H
#define FILESYNCERCLIENT_TRANSACTIONLISTENER_H

#include "../Server.h"
#include "../interfaces/Listener.h"

class TransactionListener : public Listener<TransactionOuterMsg> {
    RequestHandler<TransactionOuterMsg> *createRequestHandler(int socket) override;
    std::string getListenerName() override;
    Server *server;

public:
    TransactionListener(std::string host, int port, Server *server);
};


#endif//FILESYNCERCLIENT_TRANSACTIONLISTENER_H
