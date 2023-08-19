//
// Created by vini84200 on 8/17/23.
//

#ifndef FILESYNCERCLIENT_CLIENTCONNECTION_H
#define FILESYNCERCLIENT_CLIENTCONNECTION_H


#include "../common/Connection.h"
#include "proto/message.pb.h"

class ClientConnection : public Connection<Request, Response> {

public:
    ClientConnection(ConnectionArgs args) : Connection<Request, Response>(args) {
        int rv;
        rv = this->doLogin(args.username, args.password);
        if (rv == -1) {
            perror("Login failed");
            currConnState = ConnectionState::CONNECTION_FAILURE;
        }
        if (getSessionId() == -1) {
            printf("Login failed\n");
            currConnState = ConnectionState::CONNECTION_FAILURE;
        }
    }
    int doLogin(std::string username, std::string password);

    ClientConnection(ClientConnection &other) : Connection<Request, Response>(other) {
        this->sessionId = other.sessionId;
    }

    bool isLogged();
    int getSessionId() override;
    int sessionId = 0;
};


#endif//FILESYNCERCLIENT_CLIENTCONNECTION_H
