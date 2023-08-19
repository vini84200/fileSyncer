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

    if (r.type() == AdminMsgType::HEARTBEAT) {
        handleHeartbeat();
    }

    // Eleição aqui
}

void AdminRequestHandler::handleHeartbeat() {
    AdminMsg *msg = new AdminMsg();
    msg->set_type(AdminMsgType::HEARTBEAT);
    sendMessage(*msg);
    endConnection();
}
