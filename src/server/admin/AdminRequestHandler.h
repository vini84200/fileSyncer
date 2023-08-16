//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_ADMINREQUESTHANDLER_H
#define FILESYNCERCLIENT_ADMINREQUESTHANDLER_H

#include "../interfaces/RequestHandler.h"

class AdminRequestHandler : public RequestHandler {
public:
    void handleRequest() override;
};


#endif//FILESYNCERCLIENT_ADMINREQUESTHANDLER_H
