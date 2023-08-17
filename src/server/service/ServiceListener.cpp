//
// Created by vini84200 on 8/16/23.
//

#include "ServiceListener.h"
#include "ServiceRequestHandler.h"

RequestHandler *ServiceListener::createRequestHandler(int socket) {
    return new ServiceRequestHandler(socket, server);
}

std::string ServiceListener::getListenerName() {
    return "ServiceListener";
}

ServiceListener::ServiceListener(std::string host, int port, Server* server)
    : Listener(host,port), server(server) {
}
