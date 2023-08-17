//
// Created by vini84200 on 8/16/23.
//

#include "CreateUserTransaction.h"
#include "Transaction.h"

void CreateUserTransaction::execute() {
    ServerState &state = *getWorkState();
    if (this->username.empty() || this->password.empty()) {
        printf("Invalid username or password\n");
        rollback();
    }
    if (state.hasUser(this->username)) {
        printf("User already exists\n");
        rollback();
    }
    state.addUser(this->username, this->password);

}

CreateUserTransaction::CreateUserTransaction(
        const std::string username,
        const std::string password)
    : Transaction() {
    this->username = username;
    this->password = password;
}

TransactionMsg *CreateUserTransaction::serialize() {
    TransactionMsg *msg = new TransactionMsg();
    msg->set_transaction_id(tid);
    msg->set_type(TransactionType::CREATE_USER);
    msg->mutable_create_user()->set_username(username);
    msg->mutable_create_user()->set_password(password);
    return msg;
}

void CreateUserTransaction::deserialize(TransactionMsg *msg) {
    tid = msg->transaction_id();
    username = msg->create_user().username();
    password = msg->create_user().password();
}

std::string CreateUserTransaction::getTransactionName() {
    return "CreateUserTransaction";
}

std::string CreateUserTransaction::toString() {
    return "CreateUserTransaction: " + username + " ****";
}
