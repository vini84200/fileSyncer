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

bool existe_sync_dir(const std::string& caminho) {
    return std::filesystem::exists(caminho) && std::filesystem::is_directory(caminho);
}

void adiciona_watcher(std::string syncDirPath, int *file_descriptor, int *wd) {
    *file_descriptor = inotify_init();

    if (*file_descriptor < 0)
        std::cerr << "Erro ao inicializar o inotify" << std::endl;

    *wd = inotify_add_watch(*file_descriptor, syncDirPath.c_str(), IN_MODIFY | IN_CLOSE_WRITE);

    if (*wd < 0)
        std::cerr << "Erro ao adicionar o watch para o diretório " << syncDirPath << std::endl;
}

void verifica_modificacao(int *file_descriptor, char buffer_inotify[BUF_INOTIFY_LEN]) {
    int length = read(*file_descriptor, buffer_inotify, BUF_INOTIFY_LEN);
    if (length < 0)
        std::cerr << "Erro ao ler eventos do inotify" << std::endl;

    for (int i = 0; i < length;) {
        struct inotify_event* event = (struct inotify_event*) &buffer_inotify[i];
        if (event->len) {
            if ((event->mask & IN_MODIFY) || (event->mask & IN_CLOSE_WRITE)) {
                std::cout << "Arquivo modificado: " << event->name << std::endl;
            }
        }
        i += EVENT_SIZE + event->len;
    }
}

void fecha_watcher(int *file_descriptor, int *wd) {
    inotify_rm_watch(*file_descriptor, *wd);

    close(*file_descriptor);
}


int terminal_Interaction(char * command, char * file_path)
{

    if (strcmp(command, "help\0") == 0)
    {
        printf("\nupload <path/filename.ext>\ndownload <filename.ext>\ndelete <filename.ext> \nlist_server\nlist_client \nget_sync_dir \nexit \n\n");
        return 1;
    }
    if (strcmp(command, "get_sync_dir\0") == 0)
    {
        // chama função get_sync_dir
        return 1;
    }
    if (strcmp(command, "list_client\0") == 0)
    {
        // chama função get_sync_dir list_client
        return 1;
    }

    if (strcmp(command, "list_server\0") == 0)
    {
        // chama função list_server
        return 1;
    }

    if (strcmp(command, "exit\0") == 0)
    {
        return 0;
    }

    if (strcmp(command, "download\0") == 0)
    {
        // chama função download1;
        return 1;
    }

       if (strcmp(command, "upload\0") == 0)
    {
        // chama função upoad1;
        return 1;
    }

    if (strcmp(command, "delete\0") == 0)
    {
        // chama função delete;
        return 1;
    }

    printf("\nErro! Comando não encontrado.");
    return 0;
}

// https://beej.us/guide/bgnet/html/split-wide/client-server-background.html
int main(int argc, char *argv[]) {

    int sockfd, numbytes, wd, file_descriptor;
    char buf[MAXDATASIZE], buffer_inotify[BUF_INOTIFY_LEN];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 3)
    {
        fprintf(stderr, "usage: client hostname\n");
        char buffer[200], command[50], file_path[150];
        int notExit = 1;
        while (notExit)
        {
            printf("Digite o comando desejado ou 'help' para ver a lista de comandos\n");
            fgets(buffer, 64, stdin);
            sscanf(buffer, "%s %s", command, file_path);
            notExit = terminal_Interaction(command, file_path);
        }
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
    
    // INOTIFY 

    std::stringstream ss; 
    std::string syncDirPath;

    ss << "../../sync_dir_" << argv[1];
    ss >> syncDirPath;

    if (!existe_sync_dir(argv[1])) {
        // cria sync_dir
    }

    adiciona_watcher(syncDirPath, &file_descriptor, &wd);

    // Esse while infinito trava a tela e imprime quando algum arquivo foi criado ou modificado no diretório informado
    while (true) {
        verifica_modificacao(&file_descriptor, buffer_inotify);
    }

    // COMUNICATION

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

    fecha_watcher(&file_descriptor, &wd);

    close(sockfd);

    printf("Message sent\n");

    return 0;
}
