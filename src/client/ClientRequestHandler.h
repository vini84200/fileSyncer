#ifndef CLIENTREQUESTHANDLER_H
#define CLIENTREQUESTHANDLER_H

#include "../server/interfaces/RequestHandler.h"
#include <string>
#include <iostream>
#include <unistd.h>

class ClientRequestHandler : public RequestHandler<std::string> {
public:
    ClientRequestHandler(int socket);

protected:
    void handleRequest() override;
};

#endif // CLIENTREQUESTHANDLER_H
