#include <stdio.h>
#include <stdlib.h>
#include <bits/stdc++.h> 
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fstream>
#include <iostream>

#include <sys/inotify.h>
// #include <inotify-syscalls.h>

#include <arpa/inet.h>

#include "../common/MessageComunication.h"

#define PORT  "3490"
#define MAXDATASIZE 100
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_INOTIFY_LEN (1024 * (EVENT_SIZE + 16))

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

bool diretorioExiste(const std::string& caminho) {
    return std::filesystem::exists(caminho) && std::filesystem::is_directory(caminho);
}


// https://beej.us/guide/bgnet/html/split-wide/client-server-background.html
int main(int argc, char *argv[]) {

    int sockfd, numbytes, wd, fd;
    char buf[MAXDATASIZE], buffer_inotify[BUF_INOTIFY_LEN];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 3)
    {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[2], PORT, &hints, &servinfo) != 0)) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;

    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 1;
    }


    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    // COMUNICATION
    
    std::stringstream ss; 
    std::string stringPath;

    ss << "../../sync_dir_" << argv[1];
    ss >> stringPath;
    
    if (!diretorioExiste(stringPath)){
        std::cerr << "Diretório não localizado" << std::endl;
        return 1;
    }

    fd = inotify_init();

    if (fd < 0) {
        std::cerr << "Erro ao inicializar o inotify" << std::endl;
        return 1;
    }

    wd = inotify_add_watch(fd, stringPath.c_str(), IN_MODIFY | IN_CLOSE_WRITE);

    if (wd < 0) {
        std::cerr << "Erro ao adicionar o watch para o diretório" << std::endl;
        return 1;
    }

    // Esse while infinito trava a tela e imprime quando algum arquivo foi criado ou modificado no diretório informado
    while (true) {
        int length = read(fd, buffer_inotify, BUF_INOTIFY_LEN);
        if (length < 0) {
            std::cerr << "Erro ao ler eventos do inotify" << std::endl;
            return 1;
        }

        int i = 0;
        while (i < length) {
            struct inotify_event* event = (struct inotify_event*) &buffer_inotify[i];
            if (event->len) {
                if ((event->mask & IN_MODIFY) || (event->mask & IN_CLOSE_WRITE)) {
                    std::cout << "Arquivo modificado: " << event->name << std::endl;
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    std::fstream fb;
    fb.open("../test.pdf", std::ios::binary | std::ios::in);
    if (!fb.is_open()) {
        printf("Error opening file");
        exit(1);
    }


    std::vector<char> v;
    for (char a; fb.get(a);fb.eof())
        v.push_back(a);

    printf("Size %d\n", v.size());

    char text[] = "Hello, world!";

    Message m (v.data(), v.size());
    SealedMessage sm (m);

    void  *buffer = sm.getSealedMessagePtr();

    send(sockfd, buffer, sm.getSealedMessageSize(), 0);

    delete[] (char*)buffer;

    inotify_rm_watch(fd, wd);

    close(fd);

    close(sockfd);

    printf("Message sent\n");

    return 0;
}
