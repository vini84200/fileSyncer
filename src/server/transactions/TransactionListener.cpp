//
// Created by vini84200 on 8/16/23.
//

#include "TransactionListener.h"
#include "TransactionRequestHandler.h"

RequestHandler<TransactionOuterMsg> *
TransactionListener::createRequestHandler(int socket) {
    return new TransactionRequestHandler(socket, server);
}

std::string TransactionListener::getListenerName() {
    return "TransactionListener";
}

TransactionListener::TransactionListener(std::string host, int port,
                                         Server *server) : Listener(host, port), server(server) {
}
