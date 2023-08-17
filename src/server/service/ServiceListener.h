//
// Created by vini84200 on 8/16/23.
//

#ifndef FILESYNCERCLIENT_SERVICELISTENER_H
#define FILESYNCERCLIENT_SERVICELISTENER_H

#include "../interfaces/Listener.h"

class Server;

class ServiceListener : public Listener {
private:
    RequestHandler *createRequestHandler(int socket) override;
    std::string getListenerName() override;
    Server *server;
public:
    ServiceListener(std::string host, int port, Server *server);
};


#endif//FILESYNCERCLIENT_SERVICELISTENER_H
