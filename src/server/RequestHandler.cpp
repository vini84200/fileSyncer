//
// Created by vini84200 on 7/7/23.
//

#include "RequestHandler.h"

#include <bits/types/sigset_t.h>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <tuple>
#include <unistd.h>
#include <utility>
#include <csignal>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <filesystem>
#include <openssl/sha.h>
#include "../client/Connection.h"
#include "proto/message.pb.h"
#include "../common/utils.h"

std::string getUserFolder(std::string username) {
    return std::string(getenv("HOME")) + "/.syncer/" + username;
}

std::optional<std::pair<Header, std::string>> RequestHandler::receiveMsg() {
    std::array<char, Header::getHeaderSize()> header_buff{};
    bool ok = receiveBytes(header_buff.data(), Header::getHeaderSize());
    if (!ok) {
        return {};
    }

    Header h;
    if (h.loadFromBuffer(header_buff.data()) == -1) {
        perror("Error loading header");
        endConnection();
        return {};
    }

    std::string msg_buff;
    msg_buff.assign(h.msg_size, 0);

    if (!receiveBytes(msg_buff.data(), h.msg_size)) {
        return {};
    }

    return {{h, msg_buff}};
}

bool RequestHandler::receiveBytes(char *bytes, size_t bytes_to_receive) {
    size_t received = 0;
    size_t now = 0;
    while (received < bytes_to_receive) {
        now = recv(client_fd, bytes + received, bytes_to_receive, 0);
        if (now == CONNECTION_WAS_CLOSED) {
            return false;
        }
        if (now == SEND_ERROR) {
            return false;
        }
        received += now;
    }
    return true;
}

bool RequestHandler::sendMessage(const Message msg) {
    SealedMessage sm(msg, -1);
    char *buffer = (char*) sm.getSealedMessagePtr();
    size_t hasToSend = sm.getSealedMessageSize();
    size_t sent = 0;

    while (sent < hasToSend) {
        int sent_now = send(client_fd, buffer + sent , hasToSend - sent, 0);

        if (sent_now == CONNECTION_WAS_CLOSED) {
            return false;
        }
        if (sent_now == SEND_ERROR) {
            return false;
        }
        sent += sent_now;
    }

    return true;
}

void *run(void *args) {
    auto *rh = (RequestHandler *) args;
    rh->handleRequest();
    return nullptr;
}

pthread_t RequestHandler::start() {
    if (pthread_create(&thread_, nullptr, run, this) != 0) {
        perror("Error creating thread");
        exit(1);
    }
}

RequestHandler::RequestHandler(sockaddr_storage client_addr, int client_fd) :
        client_addr(&client_addr), client_fd(client_fd) {

}

std::map<std::string, std::string> senhas;
std::map<int, std::string> logged_user_sessions;
std::map<std::string, std::vector<std::tuple<int, RequestHandler>>> subscribed_users;


void RequestHandler::handleRequest() {
    printf("Handling request\n");
    auto msg = receiveRequest();
    if (!msg.has_value()) {
        printf("Error receiving request\n");
        endConnection();
        return;
    }
    auto h = msg.value().first;
    auto r = msg.value().second;

    switch (r.type()) {
        case PING:
            handlePing(r, h);
            printf("Ping request\n");
            break;
        case SUBSCRIBE:
            handleSubscribe(r, h);
            return;
        case DOWNLOAD:
            printf("Download request\n");
            handleDownload(r, h);
            break;
        case UPLOAD:
            printf("Upload request\n");
            break;
        case LIST:
            printf("List request\n");
            list(r, h);
            break;
        case LOGIN:
            printf("Login request\n");
            handleLogin(r, h);
            break;
        case LOGOUT:
            printf("Logout request\n");
            break;
        case FILE_UPDATE:
            fileUpdate(r, h);
            break;
        default:
            perror("Unknown request type");
            exit(1);
    }

    endConnection();
}

bool RequestHandler::endConnection() {
    close(client_fd);
}

std::optional<std::pair<Header, Request>> RequestHandler::receiveRequest() {
    auto msg = receiveMsg();
    if (!msg.has_value()) {
        return {};
    }

    auto h = msg.value().first;
    auto m = msg.value().second;

    Request r;
    if (r.ParseFromString(m) == 0) {
        return {};
    }

    return {{h, r}};
}

void RequestHandler::handleLogin(Request request, Header header) {
    if (request.type() != LOGIN) {
        perror("Invalid request type");
        exit(1);
    }

    auto login = request.username();
    auto password = request.password();

    if (senhas.count(login) == 0) {
        senhas[login] = password;
        std::filesystem::create_directory(getUserFolder(login));
        printf("New user created %s\n", login.c_str());
    }
    if (senhas[login] == password) {
        printf("Login successful\n");

        int session_id;
        srand(time(NULL));
        while (logged_user_sessions.count(session_id = rand()) != 0);
        logged_user_sessions[session_id] = login;

        Response response;
        response.set_type(LOGIN_OK);

        response.set_session_id(session_id);
        sendResponse(response);
    } else {
        printf("Login failed\n");
        Response response;
        response.set_type(ERROR);
        response.set_error_message("Invalid login or password");
        sendResponse(response);
    }
}

bool RequestHandler::sendResponse(Response response) {
    Message msg(std::move(response));
    return sendMessage(msg);
}

void RequestHandler::handlePing(Request request, Header header) {
    if (request.type() != PING) {
        perror("Invalid request type");
        exit(1);
    }

    Response response;
    response.set_type(PONG);
    sendResponse(response);
}

void RequestHandler::handleSubscribe(Request request, Header header) {
    if (logged_user_sessions.count(header.session_id) == 0) {
        Response response;
        response.set_type(ERROR);
        response.set_error_message("Invalid session id");
        sendResponse(response);
        return;
    }

    std::string user = logged_user_sessions[header.session_id];

    subscribed_users[user].emplace_back(header.session_id, *this);

}

void RequestHandler::fileUpdate(Request request, Header header) {
    if (logged_user_sessions.count(header.session_id) == 0) {
        Response response;
        response.set_type(ERROR);
        response.set_error_message("Invalid session id");
        sendResponse(response);
        return;
    }
    std::string user = logged_user_sessions[header.session_id];
    std::string filename = getUserFolder(user) + "/" + request.file_update().filename();

    // Save the new file
    if (request.file_update().deleted()) {
        std::remove(filename.c_str());
    } else {
        std::string hash_str = sha256_file(filename);
        if (hash_str == request.file_update().hash()) {
            // File already up to date
            Response response;
            response.set_type(FILE_UPDATED);
            sendResponse(response);
            return;
        }
        std::ofstream file(filename.c_str(),
                           std::ios::binary | std::ios::out | std::ios::trunc);
        file << request.file_update().data();
    }


    // Send the file to all subscribed users
    for (auto &[sid, rh]: subscribed_users[user]) {
        Response response;
        response.set_type(UPDATED);
        response.mutable_file_update()->set_filename(request.file_update().filename());
        response.mutable_file_update()->set_deleted(request.file_update().deleted());
        response.mutable_file_update()->set_data(request.file_update().data());
        response.mutable_file_update()->set_hash(request.file_update().hash());
        rh.sendResponse(response);
    }

    Response response;
    response.set_type(FILE_UPDATED);
    sendResponse(response);


}

void RequestHandler::list(Request request, Header header) {
    if (logged_user_sessions.count(header.session_id) == 0) {
        Response response;
        response.set_type(ERROR);
        response.set_error_message("Invalid session id");
        sendResponse(response);
        return;
    }
    std::string user = logged_user_sessions[header.session_id];

    Response response;
    response.set_type(FILE_LIST);
    for (auto &p: std::filesystem::directory_iterator(getUserFolder(user))) {
        auto& f = *response.mutable_file_list()->add_files();
        f.set_filename(p.path().filename());
    }
    sendResponse(response);
}

void RequestHandler::handleDownload(Request request, Header header) {
    if (logged_user_sessions.count(header.session_id) == 0) {
        Response response;
        response.set_type(ERROR);
        response.set_error_message("Invalid session id");
        sendResponse(response);
        return;
    }

    std::string user = logged_user_sessions[header.session_id];

    std::string filename = getUserFolder(user) + "/" + request.filename();
    // check if file exists and is within the user folder
    if (!std::filesystem::exists(filename) || !std::filesystem::equivalent(std::filesystem::path(filename).parent_path(), getUserFolder(user))) {
        Response response;
        response.set_type(ERROR);
        response.set_error_message("Invalid file");
        sendResponse(response);
        return;
    }

    std::fstream file(filename.c_str(), std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        Response response;
        response.set_type(ERROR);
        response.set_error_message("Could not open file");
        sendResponse(response);
        return;
    }

    Response response;
    response.set_type(DATA);
    response.mutable_file_update()->set_filename(request.filename());
    response.mutable_file_update()->set_deleted(false);
    response.mutable_file_update()->mutable_data()->assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    sendResponse(response);

}
