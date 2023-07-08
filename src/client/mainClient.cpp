#include <cstdio>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <bits/stdc++.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fstream>
#include <iostream>

#include <sys/inotify.h>
#include "../common/MessageComunication.h"
#include "Connection.h"
#include "proto/message.pb.h"
#include "vcpkg_installed/x64-linux/include/google/protobuf/stubs/common.h"

#define PORT  3490
#define MAXDATASIZE 100
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_INOTIFY_LEN (1024 * (EVENT_SIZE + 16))

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

bool existe_sync_dir(const std::string &caminho) {
    return std::filesystem::exists(caminho) && std::filesystem::is_directory(caminho);
}

void adiciona_watcher(std::string syncDirPath, int *file_descriptor, int *wd) {
    *file_descriptor = inotify_init();

    if (*file_descriptor < 0)
        std::cerr << "Erro ao inicializar o inotify" << std::endl;

    *wd = inotify_add_watch(*file_descriptor, syncDirPath.c_str(), IN_MODIFY | IN_CLOSE_WRITE);

    if (*wd < 0)
        errno == ENOENT ? std::cerr << "Diretório " << syncDirPath << " não encontrado" << std::endl
                        :
        std::cerr << "Erro ao adicionar o watch para o diretório " << syncDirPath << std::endl;
}

void verifica_modificacao(int *file_descriptor, char buffer_inotify[BUF_INOTIFY_LEN]) {
    int length = read(*file_descriptor, buffer_inotify, BUF_INOTIFY_LEN);
    if (length < 0)
        std::cerr << "Erro ao ler eventos do inotify" << std::endl;

    for (int i = 0; i < length;) {
        struct inotify_event *event = (struct inotify_event *) &buffer_inotify[i];
        if (event->len) {
            if ((event->mask & IN_MODIFY) || (event->mask & IN_CLOSE_WRITE)) {
                std::cout << "Arquivo modificado: " << event->name << std::endl;
                // TODO: Enviar arquivo modificado para o servidor
            } else {
                std::cout << "Outro evento: " << event->name << std::endl;
            }
        }
        i += EVENT_SIZE + event->len;
    }
}

void fecha_watcher(int *file_descriptor, int *wd) {
    inotify_rm_watch(*file_descriptor, *wd);

    close(*file_descriptor);
}

int terminal_Interaction(char *command, char *file_path) {

    if (strcmp(command, "help\0") == 0) {
        printf("\nupload <path/filename.ext>\ndownload <filename.ext>\ndelete <filename.ext> \nlist_server\nlist_client \nget_sync_dir \nexit \n\n");
        return 1;
    }
    if (strcmp(command, "get_sync_dir\0") == 0) {
        // chama função get_sync_dir
        return 1;
    }
    if (strcmp(command, "list_client\0") == 0) {
        // chama função get_sync_dir list_client
        return 1;
    }

    if (strcmp(command, "list_server\0") == 0) {
        // chama função list_server
        return 1;
    }

    if (strcmp(command, "exit\0") == 0) {
        return 0;
    }

    if (strcmp(command, "download\0") == 0) {
        // chama função download1;
        return 1;
    }

    if (strcmp(command, "upload\0") == 0) {
        // chama função upoad1;
        return 1;
    }

    if (strcmp(command, "delete\0") == 0) {
        // chama função delete;
        return 1;
    }

    printf("\nErro! Comando não encontrado.");
    return 0;
}

char *username;
char *password;

Connection *mainConn;

int notExit;

void *thread_monitoramento(void *arg) {
    char buffer_inotify[BUF_INOTIFY_LEN];
    int wd, file_descriptor;
    // INOTIFY

    std::stringstream ss;
    std::string syncDirPath;

    ss << getenv("HOME");
    ss << "/sync_dir_" << username;
    ss >> syncDirPath;

    if (!existe_sync_dir(syncDirPath)) {
        // cria sync_dir
        std::filesystem::create_directory(syncDirPath);
    }

    adiciona_watcher(syncDirPath, &file_descriptor, &wd);

    // Esse while infinito trava a tela e imprime quando algum arquivo foi criado ou modificado no diretório informado
    while (notExit) {
        verifica_modificacao(&file_descriptor, buffer_inotify);
    }


    fecha_watcher(&file_descriptor, &wd);
    return nullptr;
}

void *thread_updates(void *) {
    Connection &conn = *mainConn;
    Request r;
    r.set_type(RequestType::SUBSCRIBE);
    conn.sendRequest(r);
    while (conn.getConnectionState() == Connection::ConnectionState::CONNECTED && notExit) {
        auto response = conn.receiveResponse();
        if (response.has_value()) {
            auto [h, res] = response.value();
            if (res.type() == ResponseType::UPDATED) {
                std::cout << "Update: " << res.data() << std::endl;
                // TODO: Treat the new updated file
            } else {
                std::cout << "Error: Unexpected response type: " << res.type() << std::endl;
            }
        }
    }

    return nullptr;
}

void *thread_terminal(void *) {
    char buffer[200], command[50], file_path[150];
    while (notExit) {
        printf("Digite o comando desejado ou 'help' para ver a lista de comandos\n");
        fgets(buffer, 64, stdin);
        sscanf(buffer, "%s %s", command, file_path);
        notExit = terminal_Interaction(command, file_path);
    }
    return nullptr;
}


// https://beej.us/guide/bgnet/html/split-wide/client-server-background.html
int main(int argc, char *argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    if (argc != 3) {
        fprintf(stderr, "usage: fileSyncerClient username hostname\n");
        exit(1);
    }

    username = argv[1];
    // Ask for password
    password = getpass("Password: ");


    // COMUNICATION

    ConnectionArgs cArgs = ConnectionArgs(argv[2], PORT, username, password);
    mainConn = new Connection(cArgs);
    Connection &conn = *mainConn;

    if (conn.getConnectionState() != Connection::ConnectionState::CONNECTED) {
        perror("Error connecting to server");
        exit(1);
    }

    if (conn.isLogged()) {
        printf("Logged in\n");
    } else {
        printf("Wrong username or password\n");
        exit(1);
    }

    notExit = 1;

    pthread_t thread_monitoramento_id, thread_updates_id, thread_terminal_id;
    pthread_create(&thread_monitoramento_id, NULL, thread_monitoramento, NULL);
    pthread_create(&thread_updates_id, NULL, thread_updates, NULL);
    pthread_create(&thread_terminal_id, NULL, thread_terminal, NULL);

    pthread_join(thread_terminal_id, NULL);

    delete mainConn;

    return 0;
}
