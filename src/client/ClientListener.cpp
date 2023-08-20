#include "ClientListener.h"

ClientListener::ClientListener(const std::string& host, int port) 
    : Listener<std::string>(host, port) {}

RequestHandler<std::string>* ClientListener::createRequestHandler(int socket) {
    return new ClientRequestHandler(socket);
}

std::string ClientListener::getListenerName() {
    return "ClientListener";
}
