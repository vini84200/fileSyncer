#include "ClientListener.h"

ClientListener::ClientListener(const std::string& host, int port) 
    : Listener<AdminMsg>(host, port) {}

RequestHandler<AdminMsg>* ClientListener::createRequestHandler(int socket) {
    return new ClientRequestHandler(socket);
}

std::string ClientListener::getListenerName() {
    return "ClientListener";
}
