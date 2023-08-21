//
// Created by vini84200 on 16/08/23.
//

#include "AdminRequestHandler.h"
#include "../Server.h"

void AdminRequestHandler::handleRequest() {
    auto msg = receiveRequest();
    if (!msg.has_value()) {
        printf("Error receiving request\n");
        endConnection();
        return;
    }
    auto h = msg.value().first;
    auto r = msg.value().second;

    if (r.type() == AdminMsgType::HEARTBEAT) { handleHeartbeat(); }

    // Election
    if (r.type() == AdminMsgType::ELECTION) {
        handleElection(r.sender_id());
    }

    if (r.type() == AdminMsgType::ELECTION_ANSWER) {
        if (server->hasElection())
            server->getElection().receiveAnswer(r.sender_id());
    }

    if (r.type() == AdminMsgType::ELECTION_COORDINATOR) {
        if (server->hasElection()) {
            // We are in an election
            server->getElection().receiveCoordinatorMessage(
                    r.sender_id());
        } else {
            // We are not in an election
            // Set the coordinator
            server->setCoordinator(r.sender_id());
        }
    }
}

void AdminRequestHandler::handleHeartbeat() {
    AdminMsg *msg = new AdminMsg();
    msg->set_type(AdminMsgType::HEARTBEAT);
    sendMessage(*msg);
    endConnection();
}

void AdminRequestHandler::handleElection(int senderId) {
    AdminMsg *msg = new AdminMsg();
    // We reply with an answer
    msg->set_type(AdminMsgType::ELECTION_ANSWER);
    msg->set_sender_id(server->getId());
    Connection<AdminMsg, AdminMsg> connection(
            server->getReplica(senderId).getAdminConnectionArgs());
    connection.sendMessage(*msg);
    endConnection();


    if (!server->hasElection()) {
        // Start a new election
        server->startElection();
    }
}
