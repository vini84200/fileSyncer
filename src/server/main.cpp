#include <cstdio>
#include <signal.h>

#include <thread>
#include <vector>

#include <filesystem>
#include <fstream>

#include "../common/MessageComunication.h"
#include "Server.h"

#define BACKLOG 5
// Server
Server *server = nullptr;

void handle_interrupt(int a) {
    printf("\nStopping server\n");
    if (server != nullptr) { server->stop(); }
}

int main() {
    signal(SIGINT, &handle_interrupt);

    server = new Server();
    server->start();
}
