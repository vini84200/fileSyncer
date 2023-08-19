//
// Created by vini84200 on 8/19/23.
//

#include "RemoveSessionTransaction.h"

void RemoveSessionTransaction::serialize(TransactionMsg *out) {
    out->set_type(TransactionType::REMOVE_SESSION);
    out->set_transaction_id(tid);
    out->mutable_remove_session()->set_session_id(sessionID);
    printf("Serialized RemoveSessionTransaction: %d\n", sessionID);
}

void RemoveSessionTransaction::deserialize(
        const TransactionMsg *msg) {
    tid = msg->transaction_id();
    sessionID = msg->remove_session().session_id();
    printf("Deserialized RemoveSessionTransaction: %d\n", sessionID);
}

std::string RemoveSessionTransaction::getTransactionName() {
    return "RemoveSessionTransaction";
}

std::string RemoveSessionTransaction::toString() {
    return "RemoveSessionTransaction: " + std::to_string(sessionID);
}

void RemoveSessionTransaction::execute() {
    ServerState &state = *getWorkState();
    printf("RemoveSessionTransaction: %d\n", sessionID);
    if (!state.isSessionValid(sessionID)) {
        rollback();
        return;
    }
    state.removeSession(sessionID);
    state.setUserLastTid(state.getUserFromSesssion(sessionID), tid);
}

RemoveSessionTransaction::RemoveSessionTransaction(
        int32_t sessionID) : sessionID(sessionID) {

}
