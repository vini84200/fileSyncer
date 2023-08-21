//
// Created by vini84200 on 8/20/23.
//

#include "NewFrontendTransaction.h"

void NewFrontendTransaction::serialize(TransactionMsg *out) {
    out->set_type(TransactionType::NEW_FRONTEND);
    out->set_transaction_id(getTid());
    out->mutable_new_frontend()->set_hostname(hostname);
    out->mutable_new_frontend()->set_port(port);
}

void NewFrontendTransaction::deserialize(const TransactionMsg *msg) {
    hostname = msg->new_frontend().hostname();
    port     = msg->new_frontend().port();
    tid      = msg->transaction_id();
}

std::string NewFrontendTransaction::getTransactionName() {
    return "NewFrontendTransaction";
}

std::string NewFrontendTransaction::toString() {
    return "NewFrontendTransaction " + std::to_string(tid) + " " + hostname + ":" + std::to_string(port);
}

void NewFrontendTransaction::execute() {
    auto state = getWorkState();
    state->addFrontend(hostname, port);
}

NewFrontendTransaction::NewFrontendTransaction(std::string hostname,
                                               int port) {
    this->hostname = std::move(hostname);
    this->port     = port;

}
