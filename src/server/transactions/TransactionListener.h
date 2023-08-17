//
// Created by vini84200 on 8/16/23.
//

#ifndef FILESYNCERCLIENT_TRANSACTIONLISTENER_H
#define FILESYNCERCLIENT_TRANSACTIONLISTENER_H

#include "../interfaces/Listener.h"

class TransactionListener : Listener<TransactionMsg> {
    RequestHandler<TransactionMsg> *createRequestHandler(int socket) override;
    std::string getListenerName() override;
};


#endif//FILESYNCERCLIENT_TRANSACTIONLISTENER_H
