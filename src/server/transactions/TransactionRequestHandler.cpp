//
// Created by vini84200 on 8/17/23.
//

#include "TransactionRequestHandler.h"
#include "../Server.h"
#include "CreateSessionTransaction.h"
#include "CreateUserTransaction.h"
#include "FileChangeTransaction.h"
#include "RemoveSessionTransaction.h"
#include "Transaction.h"

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
        } else if (tMsg.transaction().type() == TransactionType::REMOVE_SESSION) {
            t = new RemoveSessionTransaction();
        } else if (tMsg.transaction().type() == TransactionType::FILE_CHANGE) {
            t = new FileChangeTransaction();
        } else {
            printf("Invalid transaction type received\n");
            return;
        }
        t->deserialize(&tMsg.transaction());
        printf("Received transaction (id: %d)\n", t->getTid());
        TransactionOuterMsg msg;
        msg.set_type(TransactionOuterType::TRANSACTION_OK_ACK);
        if(!sendMessage(msg)) {
            printf("Error sending ACK message to client\n");
            return;
        } else {
            printf("ACK message sent\n");
        }
        server->getTransactionManager().receiveNewTransaction(*t);
        printf("Message sent\n");
    }
    if (tMsg.type() == TransactionOuterType::TRANSACTION_VOTE) {
        // This is the coordinator
        printf("Received vote (id: %d): %s\n", tMsg.vote().transaction_id(), tMsg.vote().vote() == TransactionVote::COMMIT ? "COMMIT" : "ROLLBACK");
        server->getTransactionManager().addVote(tMsg.vote().vote() == TransactionVote::COMMIT, tMsg.vote().transaction_id());
        TransactionOuterMsg msg;
        msg.set_type(TransactionOuterType::TRANSACTION_OK_ACK);
        sendMessage(msg);
        printf("Message sent\n");
        return;

    }
    if (tMsg.type() == TransactionOuterType::TRANSACTION_RESULT) {
        // This is the coordinator
        printf("Received result (id: %d): %s\n", tMsg.result().transaction_id(), tMsg.result().result() == TransactionVote::COMMIT ? "COMMIT" : "ROLLBACK");
        server->getTransactionManager().receiveResult(tMsg.result().result() == TransactionVote::COMMIT, tMsg.result().transaction_id());
        TransactionOuterMsg msg;
        msg.set_type(TransactionOuterType::TRANSACTION_OK_ACK);
        sendMessage(msg);
        printf("Message sent\n");
        return;
    }


}
