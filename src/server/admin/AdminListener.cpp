//
// Created by vini84200 on 16/08/23.
//

#include "AdminListener.h"
#include "AdminRequestHandler.h"

RequestHandler<AdminMsg> *AdminListener::createRequestHandler(int socket) {
    return new AdminRequestHandler(socket, server);
}

std::string AdminListener::getListenerName() {
    return "AdminListener";
}

AdminListener::AdminListener(std::string host, int ip, Server *server)
    : Listener(host, ip) {
    this->server = server;
}
