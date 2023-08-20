//
// Created by vini84200 on 8/19/23.
//

#ifndef FILESYNCERCLIENT_FILECHANGETRANSACTION_H
#define FILESYNCERCLIENT_FILECHANGETRANSACTION_H

#include "Transaction.h"

class FileChangeTransaction : public Transaction {
public:
    FileChangeTransaction() = default;
    FileChangeTransaction(std::string username, std::string filename,
                          bool deleted, byte_string *content);
    ~FileChangeTransaction() ;
    void serialize(TransactionMsg *out) override;
    void deserialize(const TransactionMsg *msg) override;
    std::string getTransactionName() override;
    std::string toString() override;

protected:
    void commitHook() override;
    void rollbackHook() override;

protected:
    void execute() override;
    std::string username;
    std::string filename;
    byte_string *content;
    bool deleted;
    bool deleteOnExit = false;
};


#endif//FILESYNCERCLIENT_FILECHANGETRANSACTION_H
