//
// Created by vini84200 on 8/16/23.
//

#include "CreateUserTransaction.h"
#include "Transaction.h"
#include <filesystem>
#include <fstream>

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
    state.setUserLastTid(this->username, tid);
    // Create the user directory
    std::string userDir = state.getUserDir(this->username);
    // Create the user directory as a temporary directory
    std::filesystem::create_directory(userDir + "~");
    // Create the user password file
    {
        std::ofstream passwordFile(userDir + "~/.password");
        passwordFile << this->password;
    }

}

CreateUserTransaction::CreateUserTransaction(
        const std::string username,
        const std::string password)
    : Transaction() {
    this->username = username;
    this->password = password;
}

void CreateUserTransaction::serialize(TransactionMsg *out) {
    out->set_transaction_id(tid);
    out->set_type(TransactionType::CREATE_USER);
    out->mutable_create_user()->set_username(username);
    out->mutable_create_user()->set_password(password);
}

void CreateUserTransaction::deserialize(const TransactionMsg *msg) {
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

void CreateUserTransaction::commitHook() {
    // Rename the temporary directory to the actual directory
    std::string userDir = getWorkState()->getUserDir(this->username);
    std::filesystem::rename(userDir + "~", userDir);
}

void CreateUserTransaction::rollbackHook() {
    // Delete the temporary directory
    std::string userDir = getWorkState()->getUserDir(this->username);
    std::filesystem::remove_all(userDir + "~");
}
