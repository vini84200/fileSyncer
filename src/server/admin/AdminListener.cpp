//
// Created by vini84200 on 16/08/23.
//

#include "AdminListener.h"

RequestHandler *AdminListener::createRequestHandler(int socket) {
}

std::string AdminListener::getListenerName() {
    return "AdminListener";
}

AdminListener::AdminListener(std::string host, int ip, Server *server)
    : Listener(host, ip) {
    this->server = server;
}
