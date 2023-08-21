#include "../server/interfaces/RequestHandler.h"
#include "ClientRequestHandler.h"
#include <string>
#include <iostream>
#include <unistd.h>

void changeIP(std::string hostname, int port);

void ClientRequestHandler::handleRequest() {
    auto msg = receiveRequest();
    if (!msg.has_value()) {
        printf("Error receiving request\n");
        endConnection();
        return;
    }
    auto h = msg.value().first;
    auto r = msg.value().second;

    if (r.type() == AdminMsgType::NEW_COORDINATOR) { changeIP(r.coordinatorinfo().hostname(), r.coordinatorinfo().port()); }
}