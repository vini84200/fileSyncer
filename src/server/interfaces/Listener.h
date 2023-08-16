//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_LISTENER_H
#define FILESYNCERCLIENT_LISTENER_H

#include "RequestHandler.h"
#include <csignal>
#include <string>

class Listener {
public:
    Listener(std::string host, int port);
    void start();
    void stop();

    static void *run(void *args);
    void execute();

protected:
    virtual RequestHandler *createRequestHandler(int socket) = 0;
    virtual std::string getListenerName()                    = 0;

private:
    pthread_t thread_id;
    std::vector<RequestHandler *> handlers;
    std::string host;
    int port;
    bool is_running = true;
    int socket_fd;
};


#endif//FILESYNCERCLIENT_LISTENER_H
