#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <thread>
#include <unistd.h>
#include <vector>

#include <filesystem>
#include <fstream>
#include <iostream>

#include "../common/MessageComunication.h"
#include "Server.h"
#include "service/ServerRequestHandler.h"

#define BACKLOG 5
// Server
Server *server = nullptr;

void handle_interrupt(int a) {
    printf("\nStopping server\n");
    if (server != nullptr) { server->stop(); }
}

int main() {
    std::filesystem::create_directory(std::string(getenv("HOME")) +
                                      "/.syncer");

    signal(SIGINT, &handle_interrupt);

    server = new Server();
    server->start();
}
