//
// Created by vini84200 on 8/16/23.
//

#ifndef FILESYNCERCLIENT_SERVICEREQUESTHANDLER_H
#define FILESYNCERCLIENT_SERVICEREQUESTHANDLER_H

#include "../Server.h"
#include "../interfaces/RequestHandler.h"

class ServiceRequestHandler : public RequestHandler<Request> {
public:
    ServiceRequestHandler(int socket, Server *server);
    void handleRequest() override;

    void handleLogin(Request request, Header header);
    Server *server;
    void handleSubscribe(Request request, std::string user,
                         Header header);
    void handleDownload(Request request, std::string user,
                        Header header);
    void handleList(Request request, std::string user, Header header);
    void handleLogout(Request request, std::string user,
                      Header header);
    void handleFileUpdate(Request request, std::string user,
                          Header header);
    void handleUpload(Request request, std::string user,
                      Header header);
};


#endif//FILESYNCERCLIENT_SERVICEREQUESTHANDLER_H
