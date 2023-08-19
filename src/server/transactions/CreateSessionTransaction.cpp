//
// Created by vini84200 on 8/16/23.
//

#include "CreateSessionTransaction.h"
#include "proto/message.pb.h"

void CreateSessionTransaction::execute() {
    ServerState& state = *getWorkState();
    if (state.isSessionValid(sessionID)) {
        // Session already exists
        rollback();
    }
    if (!state.hasUser(username)) {
        // User does not exist
        rollback();
    }
    state.addSession(sessionID, username);
    state.setUserLastTid(username, tid);
}

CreateSessionTransaction::CreateSessionTransaction(
        int sessionID, std::string username) {
    this->sessionID = sessionID;
    this->username = std::move(username);
}

void *CreateSessionTransaction::serialize(TransactionMsg *msg) {
    msg->set_transaction_id(getTid());
    msg->set_type(TransactionType::CREATE_SESSION);
    msg->mutable_create_session()->set_session_id(sessionID);
    msg->mutable_create_session()->set_username(username);
}

void CreateSessionTransaction::deserialize(
        const TransactionMsg *msg) {
    tid = msg->transaction_id();
    sessionID = msg->create_session().session_id();
    username = msg->create_session().username();
}

std::string CreateSessionTransaction::getTransactionName() {
    return "CreateSessionTransaction";
}

std::string CreateSessionTransaction::toString() {
    return "CreateSessionTransaction: " + std::to_string(sessionID) + " " + username;
}
