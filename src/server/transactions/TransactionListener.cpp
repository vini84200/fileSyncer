//
// Created by vini84200 on 8/16/23.
//

#include "TransactionListener.h"

RequestHandler *
TransactionListener::createRequestHandler(int socket) {
    return nullptr;
}

std::string TransactionListener::getListenerName() {
    return "TransactionListener";
}
