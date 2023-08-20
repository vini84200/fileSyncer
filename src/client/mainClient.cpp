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

#include "../common/MessageComunication.h"
#include "../common/utils.h"
#include "ClientConnection.h"
#include "proto/message.pb.h"
#include "ClientListener.h"
#include <openssl/sha.h>
#include <sys/inotify.h>
#define PORT 8989
#define DAEMON_PORT 666
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_INOTIFY_LEN (1024 * (EVENT_SIZE + 16))

#define CONTINUE 1

std::string syncDirPath;
char *username;
char *password;

ClientConnection *mainConn;

int notExit;
bool muteUpdate = false;


int getSyncDir();

int listClient();

int listServer();

int ping();

int download(char *path);

int upload(char *path);

int deleteFile(char *path);

void sendLogout();

int showHostIp();

void changeIP(std::string hostname, int port);

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

    *wd = inotify_add_watch(*file_descriptor, syncDirPath.c_str(), IN_MODIFY | IN_CLOSE_WRITE | IN_DELETE);

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
            if ((event->mask & 0) || (event->mask & IN_CLOSE_WRITE) ){
                if (muteUpdate) {
//                    printf("Arquivo atualizado, processo de sincronização em andamento\n");
                    continue;
                }
                std::cout << "Arquivo modificado: " << event->name << std::endl;
                ClientConnection conn(*mainConn);
                Request request;
                request.set_type(RequestType::FILE_UPDATE);
                const std::string filename = event->name;
                request.mutable_file_update()->set_filename(filename);
                request.mutable_file_update()->set_deleted(false);
                std::string newHash = digest_to_string(getFileDigest(syncDirPath + "/" + filename));
                request.mutable_file_update()->set_hash(newHash);
                // Send the hash to the server and wait for a response
                conn.sendRequest(request);
                auto maybeResponse = conn.receiveResponse();
                if (!maybeResponse.has_value()) {
                    std::cerr << "Erro ao receber resposta do servidor" << std::endl;
                    return;
                }

                auto [header, response] = maybeResponse.value();

                if (response.type() == ERROR) {
                    std::cerr << "Erro ao enviar arquivo para o servidor" << std::endl;
                    return;
                }

                if (response.type() == FILE_UPDATED) {
                    std::cout << "Arquivo já atualizado no servidor" << std::endl;
                    return;
                }

                if (response.type() == SEND_FILE_DATA) {
                    // Send the file data
                    std::fstream file;
                    file.open(syncDirPath + "/" + filename, std::ios::in | std::ios::binary);
                    Request fileDataRequest;
                    fileDataRequest.set_type(RequestType::FILE_DATA);
                    fileDataRequest.mutable_file_data_update()->mutable_data()->assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
                    fileDataRequest.mutable_file_data_update()->mutable_file_update()->set_filename(filename);
                    fileDataRequest.mutable_file_data_update()->mutable_file_update()->set_deleted(false);
                    fileDataRequest.mutable_file_data_update()->mutable_file_update()->set_hash(newHash);
                    conn.sendRequest(fileDataRequest);

                    auto maybeResponse = conn.receiveResponse();
                    if (!maybeResponse.has_value()) {
                        std::cerr << "Erro ao receber resposta do servidor" << std::endl;
                        return;
                    }
                    auto [header, response] = maybeResponse.value();
                    if (response.type() == ERROR) {
                        std::cerr << "Erro ao enviar arquivo para o servidor" << std::endl;
                        return;
                    }
                    if (response.type() == FILE_UPDATED) {
                        std::cout << "Arquivo enviado ao servidor com sucesso" << std::endl;
                    }
                }
            } else if (event->mask & IN_DELETE) {
                deleteFile((char *) event->name);
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
        return CONTINUE;
    }
    if (strcmp(command, "get_sync_dir\0") == 0) {
        return getSyncDir();
    }
    if (strcmp(command, "list_client\0") == 0) {
        return listClient();
    }

    if (strcmp(command, "list_server\0") == 0) {
        return listServer();
    }

    if (strcmp(command, "exit\0") == 0) {
        sendLogout();
        return EXIT_SUCCESS;
    }

    if (strcmp(command, "download\0") == 0) {
        return download(file_path);
    }

    if (strcmp(command, "upload\0") == 0) {
        return upload(file_path);
    }

    if (strcmp(command, "delete\0") == 0) {
        return deleteFile(file_path);
    }

    if (strcmp(command, "ping\0") == 0) {
        return ping();
    }

    if (strcmp(command, "show_host_ip\0") == 0){
        return showHostIp();
    }

    printf("\nErro! Comando não encontrado.");
    return CONTINUE;
}

void sendLogout() {
    ClientConnection conn(*mainConn);
    Request request;
    request.set_type(RequestType::LOGOUT);
    conn.sendRequest(request);
    auto maybeResponse = conn.receiveResponse();
    if (!maybeResponse.has_value()) {
        printf("\nErro ao receber resposta do servidor.");
        return;
    }
    auto [header, response] = maybeResponse.value();
    if (response.type() == ResponseType::ERROR) {
        printf("\nErro ao fazer logout: ", response.error_msg().c_str());
        return;
    }
    if (response.type() == ResponseType::LOGIN_OK) {
        printf("\nLogout realizado com sucesso.");
        return;
    }
}

int deleteFile(char *path) {
    Request request;
    request.set_type(RequestType::FILE_UPDATE);
    const std::string filename = path;
    request.mutable_file_update()->set_filename(filename);
    request.mutable_file_update()->set_deleted(true);
    request.mutable_file_update()->set_hash("");
    ClientConnection conn(*mainConn);
    conn.sendRequest(request);
    auto maybeResponse = conn.receiveResponse();
    if (!maybeResponse.has_value()) {
        std::cerr << "Erro ao receber resposta do servidor" << std::endl;
        return CONTINUE;
    }
    auto [header, response] = maybeResponse.value();
    if (response.type() == ERROR) {
        std::cerr << "Erro ao enviar arquivo para o servidor" << std::endl;
        return CONTINUE;
    }
    if (response.type() == FILE_UPDATED) {
        std::cout << "Arquivo deletado no servidor" << std::endl;
    }

    return CONTINUE;
}

int upload(char *path) {
    Request request;
    request.set_type(RequestType::UPLOAD);
    const std::string filename = path;

    // Calcula o hash do arquivo
    FileDigest hash = getFileDigest(filename);


    std::fstream file;
    file.open(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo " << filename << std::endl;
        return CONTINUE;
    }
    request.mutable_file_data_update()->set_data(std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()));
    request.mutable_file_data_update()->mutable_file_update()->set_filename(filename);
    request.mutable_file_data_update()->mutable_file_update()->set_deleted(false);
    request.mutable_file_data_update()->mutable_file_update()->set_hash(digest_to_string(hash));

    ClientConnection conn(*mainConn);
    conn.sendRequest(request);
    auto maybeResponse = conn.receiveResponse();
    if (!maybeResponse.has_value()) {
        std::cerr << "Erro ao receber resposta do servidor" << std::endl;
        return CONTINUE;
    }
    auto [header, response] = maybeResponse.value();
    if (response.type() == ERROR) {
        std::cerr << "Erro ao enviar arquivo para o servidor" << std::endl;
        return CONTINUE;
    }
    if (response.type() == FILE_UPDATED) {
        std::cout << "Arquivo enviado ao servidor com sucesso" << std::endl;
    }
    return CONTINUE;
}

int download(char *path) {
    Request request;
    request.set_type(RequestType::DOWNLOAD);
    request.set_filename(path);
    ClientConnection conn(*mainConn);
    conn.sendRequest(request);
    auto maybeResponse = conn.receiveResponse();
    if (!maybeResponse.has_value()) {
        printf("\nErro ao receber resposta do servidor.");
        return CONTINUE;
    }
    auto [header, response] = maybeResponse.value();
    if (response.type() == ResponseType::ERROR) {
        printf("\nErro ao baixar arquivo do servidor: %s\n", response.error_msg().c_str());
        return CONTINUE;
    }

    if (response.type() == ResponseType::FILE_DATA_R) {
        std::fstream file;
        file.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            printf("\nErro ao abrir o arquivo %s", path);
            return CONTINUE;
        }
        file << response.file_data().data();
        file.close();
        printf("\nArquivo %s baixado com sucesso.", path);
        return CONTINUE;
    }

    return CONTINUE;
}

int ping() {
    Connection conn(*mainConn);
    Request request;
    request.set_type(RequestType::PING);
    conn.sendRequest(request);
    auto maybeResponse = conn.receiveResponse();
    if (!maybeResponse.has_value()) {
        printf("\nErro ao receber resposta do servidor.");
        return CONTINUE;
    }
    auto [header, response] = maybeResponse.value();
    if (response.type() == ResponseType::ERROR) {
        printf("\nErro ao listar arquivos no servidor: ", response.error_msg().c_str());
        return CONTINUE;
    }
    if (response.type() == ResponseType::PONG) {
        printf("\nServidor respondeu ao ping.\n");
        return CONTINUE;
    }
}

int listServer() {
    ClientConnection conn (*mainConn);
    Request request;
    request.set_type(RequestType::LIST);
    conn.sendRequest(request);
    auto maybeResponse = conn.receiveResponse();
    if (!maybeResponse.has_value()) {
        printf("\nErro ao receber resposta do servidor.");
        return CONTINUE;
    }
    auto [header, response] = maybeResponse.value();
    if (response.type() == ResponseType::ERROR) {
        printf("\nErro ao listar arquivos no servidor: ", response.error_msg().c_str());
        return CONTINUE;
    }

    if (response.type() == ResponseType::FILE_LIST) {
        printf("\nArquivos no servidor: ");
        for (const auto &file : response.file_list().files()) {
            // TODO: mostrar data de modificação e outras informações
            printf("\n%s", file.filename().c_str());
        }
        printf("\n");
        return CONTINUE;
    }
}

int listClient() {
    printf("\nArquivos no diretório de sincronização: ");
    for ( const auto & entry : std::filesystem::directory_iterator(syncDirPath) )
        printf("\n%s", entry.path().filename().c_str());
    printf("\n");
    return CONTINUE;
}

int getSyncDir() {
    if (!existe_sync_dir(syncDirPath)) {
        std::filesystem::create_directory(syncDirPath);
        printf("\nDiretório criado com sucesso!");
    }
    printf("\nDiretório: %s", syncDirPath.c_str());
    return CONTINUE;
}

int showHostIp() {
    printf("IP: %s, port: %d\n", mainConn->hostname.c_str(), mainConn->port);
    return CONTINUE;
}

void *thread_monitoramento(void *arg) {
    char buffer_inotify[BUF_INOTIFY_LEN];
    int wd, file_descriptor;
    // INOTIFY

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
    ClientConnection conn(*mainConn);
    Request r;
    r.set_type(RequestType::SUBSCRIBE);
    conn.sendRequest(r);
    while (conn.getConnectionState() == ConnectionState::CONNECTED && notExit) {
        auto response = conn.receiveResponse();
        if (response.has_value()) {
            auto [h, res] = response.value();
            if (res.type() == ResponseType::UPDATED) {
                std::cout << "Update: " << res.file_update().filename() << std::endl;
                std::string path = syncDirPath + "/" + res.file_update().filename();
                muteUpdate = true;
                if (res.file_update().deleted()) {
                    std::remove(path.c_str());
                } else {
                    // Check the hash
                    FileDigest hash = getFileDigest(path);

                    if (digest_to_string(hash) == res.file_update().hash()) {
                        std::cout << "File already up to date" << std::endl;
                        muteUpdate = false;
                        continue;
                    }
                    // We need to download the file
                    {
                        ClientConnection conn(*mainConn);
                        Request request;
                        request.set_type(RequestType::DOWNLOAD);
                        request.set_filename(res.file_update().filename());
                        conn.sendRequest(request);
                        auto maybeResponse = conn.receiveResponse();
                        if (!maybeResponse.has_value()) {
                            std::cerr << "Erro ao receber resposta do servidor" << std::endl;
                            muteUpdate = false;
                            continue;
                        }
                        auto [header, response] = maybeResponse.value();
                        if (response.type() == ResponseType::ERROR) {
                            std::cerr << "Erro ao baixar arquivo do servidor: " << response.error_msg() << std::endl;
                            muteUpdate = false;
                            continue;
                        }

                        if (response.type() == ResponseType::FILE_DATA_R) {
                            std::fstream file;
                            file.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
                            if (!file.is_open()) {
                                std::cerr << "Erro ao abrir o arquivo " << path << std::endl;
                                muteUpdate = false;
                                continue;
                            }
                            file << response.file_data().data();
                            file.close();
                            std::cout << "File downloaded successfully" << std::endl;
                        }

                    }
                }
                muteUpdate = false;
            } else {
                std::cout << "Error: Unexpected response type 2: " << res.type() << std::endl;
            }
        }
        else {
            std::cout << "Error: Unexpected response 1" << std::endl;
        }
    }

    return nullptr;
}

void *thread_front_end(void *) {
    char buffer[200], command[50], file_path[150];
    while (notExit) {
        printf("Digite o comando desejado ou 'help' para ver a lista de comandos\n > ");
        fgets(buffer, 64, stdin);
        sscanf(buffer, "%s %s", command, file_path);
        notExit = terminal_Interaction(command, file_path);
    }
    return nullptr;
}

void changeIP(std::string hostname, int port){
    int sessionIdTemp = mainConn->sessionId;

    delete mainConn;

    ConnectionArgs cArgs = ConnectionArgs(hostname, port, username, password, sessionIdTemp);
    mainConn = new ClientConnection(cArgs);
    ClientConnection &conn = *mainConn;

    printf("IP changed to %s:%d", hostname, port);
}

void *thread_daemon_listener(void *) {
    ClientListener daemonListener("", DAEMON_PORT);

    printf("Daemon: ");
    daemonListener.start();
    
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
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
    mainConn = new ClientConnection(cArgs);
    ClientConnection &conn = *mainConn;

    if (conn.getConnectionState() != ConnectionState::CONNECTED) {
        perror("Error connecting to server");
        exit(1);
    }

    if (conn.isLogged()) {
        printf("Logged in\n");
    } else {
        printf("Wrong username or password\n");
        exit(1);
    }

    std::stringstream ss;

    ss << getenv("HOME");
    ss << "/sync_dir_" << username;
    ss >> syncDirPath;
    notExit = 1;

    pthread_t thread_monitoramento_id, thread_updates_id, thread_front_end_id, thread_daemon_listener_id;
    pthread_create(&thread_monitoramento_id, NULL, thread_monitoramento, NULL);
    pthread_create(&thread_updates_id, NULL, thread_updates, NULL);
    pthread_create(&thread_front_end_id, NULL, thread_front_end, NULL);
    pthread_create(&thread_daemon_listener_id, NULL, thread_daemon_listener, NULL);

    pthread_join(thread_front_end_id, NULL);

    delete mainConn;

    return 0;
}
