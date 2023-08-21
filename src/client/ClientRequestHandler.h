#ifndef CLIENTREQUESTHANDLER_H
#define CLIENTREQUESTHANDLER_H

#include "../server/interfaces/RequestHandler.h"
#include <string>
#include <iostream>
#include <unistd.h>

class ClientRequestHandler : public RequestHandler<AdminMsg> {
public:
    ClientRequestHandler(int socket) : RequestHandler(socket) {};

protected:
    void handleRequest() override;
};

#endif // CLIENTREQUESTHANDLER_H
