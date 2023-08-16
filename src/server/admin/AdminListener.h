//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_ADMINLISTENER_H
#define FILESYNCERCLIENT_ADMINLISTENER_H


#include "../Server.h"
#include "../interfaces/Listener.h"

class AdminListener : public Listener {
protected:
    RequestHandler *createRequestHandler(int socket) override;
    std::string getListenerName() override;

public:
    AdminListener(std::string host, int ip, Server *server);
    Server *server;
};


#endif//FILESYNCERCLIENT_ADMINLISTENER_H
