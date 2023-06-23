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

#include "MessageComunication.h"
#define MYPORT "3490"
#define BACKLOG 5

#define MSG_MAXSIZE 1024 * 1024

// Server

bool run = true;
int sockfd;

void handle_interrupt(int a) {
    printf("\nFinishing my bussiness\n");

    run = 0;


    close(sockfd);
}

struct ClientInfo {
    struct sockaddr_storage their_addr;
    int socket_fd;
    int session = 0;
};

void client_handler(ClientInfo info) {
    char buffer[MSG_MAXSIZE];
    int total_msg_rcvd = 0;
    int headerSize = Header().getHeaderSize();

    while (total_msg_rcvd < headerSize) {
        int thisrcv = recv(info.socket_fd, buffer+total_msg_rcvd, (headerSize - total_msg_rcvd), 0);
        if (thisrcv == 0) {
            printf("Cliente fechou a conexão\n");
            close(info.socket_fd);
            return;
        }
        if (thisrcv == -1) {
            printf("Erro recebendo mensagem do client\n");
            close(info.socket_fd);
            return;
        }
        total_msg_rcvd += thisrcv;
        printf("Received %d bytes\n", thisrcv);
        printf("Total received %d bytes\n", total_msg_rcvd);
        printf("Faltam %d bytes\n", headerSize - total_msg_rcvd);
    }

    Header h;
    if (h.loadFromBuffer(buffer) == -1) {
        printf("Erro lendo header\n");
        close(info.socket_fd);
        return;
    }

    printf("Msg size %d\n ", h.msg_size);

    int need_to_read = h.msg_size + headerSize;

    while (total_msg_rcvd < (h.msg_size+headerSize)) {
        int thisrcv = recv(info.socket_fd, buffer + total_msg_rcvd, (need_to_read - total_msg_rcvd), 0);
        if (thisrcv == 0) {
            printf("Cliente fechou a conexão\n");
            close(info.socket_fd);
            return;
        }
        if (thisrcv == -1) {
            printf("Erro recebendo mensagem do client\n");
            close(info.socket_fd);
            return;
        }
        total_msg_rcvd += thisrcv;
    }

    printf("Received %d bytes\n", total_msg_rcvd);

    SealedMessage sm = SealedMessage::getFromBuffer(buffer);
    Message m = sm.getMessage();
    printf("Received message: %s\n", m.getBuffer());

    // Answer

    //send();

    close(info.socket_fd);


}

// https://beej.us/guide/bgnet/html/split-wide/client-server-background.html

int main() {
    struct addrinfo hints, *res;


    signal(SIGINT, &handle_interrupt);

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo("127.0.0.1", MYPORT, &hints, &res);

    // make a socket:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // bind it to the port we passed in to getaddrinfo():

    bind(sockfd, res->ai_addr, res->ai_addrlen);


    listen(sockfd, BACKLOG);


    {
        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(res->ai_family, &((struct sockaddr_in *)res->ai_addr)->sin_addr, ipstr, sizeof ipstr);

        printf("Listening on %s:%s\n", ipstr, MYPORT);
    }



    //struct sockaddr_storage their_addr;
    socklen_t addr_size;

    std::vector<std::thread> allThreads;


    while (run) {
        ClientInfo info;

        // Accept the connections and then create a thread
        addr_size = sizeof info.their_addr;
        info.socket_fd = accept(sockfd, (struct sockaddr *) &info.their_addr, &addr_size);
        if (!run) break;
        if (info.socket_fd == -1) {
            printf("Error on accepting the connection\n");

        }
        printf("New client");
        // Pass to new thread
        allThreads.emplace_back(&client_handler, info);
    }


    printf("Closed the socket\n");

    printf("Awaiting the clients to end the communication.\n");

    for (auto & t : allThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    printf("Done. Exiting...\n");
    return 0;

}
