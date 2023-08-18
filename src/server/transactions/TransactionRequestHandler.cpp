//
// Created by vini84200 on 8/17/23.
//

#include "TransactionRequestHandler.h"
#include "CreateSessionTransaction.h"
#include "CreateUserTransaction.h"
#include "Transaction.h"
#include "../Server.h"

void TransactionRequestHandler::handleRequest() {
    auto msg = receiveRequest();
    if (!msg.has_value()) {
        printf("Error receiving request\n");
        endConnection();
        return;
    }
    auto h = msg.value().first;
    auto tMsg = msg.value().second;


    if (tMsg.type() == TransactionOuterType::TRANSACTION) {

        Transaction *t;
        if (tMsg.transaction().type() == TransactionType::CREATE_USER) {
            t = new CreateUserTransaction();
        } else if (tMsg.transaction().type() == TransactionType::CREATE_SESSION) {
            t = new CreateSessionTransaction();
        }
        t->deserialize(&tMsg.transaction());
        server->getTransactionManager().receiveNewTransaction(*t);
    }
    if (tMsg.type() == TransactionOuterType::TRANSACTION_VOTE) {
        // This is the coordinator
        server->getTransactionManager().addVote(tMsg.vote().vote() == TransactionVote::COMMIT, tMsg.vote().transaction_id());

    }
    if (tMsg.type() == TransactionOuterType::TRANSACTION_RESULT) {
        // This is the coordinator
        server->getTransactionManager().receiveResult(tMsg.result().result() == TransactionVote::COMMIT, tMsg.result().transaction_id());
    }


}
