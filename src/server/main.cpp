#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <signal.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <thread>
#include <unistd.h>
#include <vector>

#include <iostream>
#include <fstream>

#include "../common/MessageComunication.h"
#include "RequestHandler.h"

#define MYPORT "3490"
#define BACKLOG 5

#define MSG_MAXSIZE 1024 * 1024 * 24

// Server

bool run = true;
int sockfd;

void handle_interrupt(int a) {
    printf("\nFinishing my bussiness\n");
    run = 0;
    close(sockfd);
}

// https://beej.us/guide/bgnet/html/split-wide/client-server-background.html

int main() {
    struct addrinfo hints, *res;


    signal(SIGINT, &handle_interrupt);

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, MYPORT, &hints, &res);

    // make a socket:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // bind it to the port we passed in to getaddrinfo():

    bind(sockfd, res->ai_addr, res->ai_addrlen);


    listen(sockfd, BACKLOG);


    {
        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(res->ai_family, &((struct sockaddr_in *) res->ai_addr)->sin_addr, ipstr, sizeof ipstr);

        printf("Listening on %s:%s\n", ipstr, MYPORT);
    }



    //struct sockaddr_storage their_addr;
    socklen_t addr_size;

    std::vector<RequestHandler> allHandlers;
    std::vector<pthread_t> allThreads;


    while (run) {

        // Accept the connections and then create a thread
        sockaddr_storage their_addr;
        addr_size = sizeof(sockaddr_storage);
        int socket_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size);
        if (!run) break;
        if (socket_fd == -1) {
            printf("Error on accepting the connection\n");
        }
        allHandlers.emplace_back(their_addr, socket_fd);
        allThreads.emplace_back(allHandlers.back().start());

    }


    printf("Closed the socket\n");

    printf("Awaiting the clients to end the communication.\n");

    for (auto &t: allThreads) {
        pthread_join(t, NULL);
    }

    printf("Done. Exiting...\n");
    return 0;

}
