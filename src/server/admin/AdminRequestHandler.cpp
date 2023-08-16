//
// Created by vini84200 on 16/08/23.
//

#include "AdminRequestHandler.h"

void AdminRequestHandler::handleRequest() {
    auto msg = receiveRequest();
    if (!msg.has_value()) {
        printf("Error receiving request\n");
        endConnection();
        return;
    }
    auto h = msg.value().first;
    auto r = msg.value().second;
}
