//
// Created by vini84200 on 16/08/23.
//

#include "Listener.h"
#include <netdb.h>

#define BACKLOG 5

Listener::Listener(std::string host, int port) {
    this->host = std::move(host);
    this->port = port;
}

void Listener::start() {
    pthread_create(&thread_id, nullptr, run, this);
}

void *Listener::run(void *args) {
    auto *listener = (Listener *) args;
    listener->execute();
    return nullptr;
}

void Listener::execute() {
    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // Get address info
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;// use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(),
                    &hints, &res) != 0) {
        perror("ERROR on getaddrinfo");
        exit(1);
    }

    // Bind socket
    if (bind(socket_fd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    // Listen
    listen(socket_fd, BACKLOG);


    {
        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(res->ai_family,
                  &((struct sockaddr_in *) res->ai_addr)->sin_addr,
                  ipstr, sizeof ipstr);

        printf("Listening on %s:%s for %s\n", ipstr,
               std::to_string(port).c_str(),
               getListenerName().c_str());
    }
    freeaddrinfo(res);

    // Accept connections
    while (is_running) {
        socklen_t addr_size;
        sockaddr_storage their_addr{};
        addr_size = sizeof(sockaddr_storage);
        int client_socket_fd =
                accept(socket_fd, (struct sockaddr *) &their_addr,
                       &addr_size);
        if (!is_running) break;
        if (client_socket_fd == -1) {
            perror("ERROR on accepting the connection");
            continue;
        }

        auto *handler = createRequestHandler(client_socket_fd);
        handlers.push_back(handler);
    }
    close(socket_fd);
}

void Listener::stop() {
    is_running = false;

    // Close Socket
    close(socket_fd);

    // Stop all handlers
    for (auto handler: handlers) { handler->stop(); }
    pthread_join(thread_id, nullptr);
}
