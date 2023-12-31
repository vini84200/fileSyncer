//
// Created by vini84200 on 8/16/23.
//

#include "ServiceRequestHandler.h"
#include "../transactions/CreateSessionTransaction.h"
#include "../transactions/CreateUserTransaction.h"
#include "../transactions/FileChangeTransaction.h"
#include "../transactions/NewFrontendTransaction.h"
#include "../transactions/RemoveSessionTransaction.h"
#include <fstream>

ServiceRequestHandler::ServiceRequestHandler(int socket,
                                             Server *server)
    : RequestHandler(socket), server(server) {

}

void ServiceRequestHandler::handleRequest() {
    printf("Handling request\n");
    auto msg = receiveRequest();
    if (!msg.has_value()) {
        printf("Error receiving request\n");
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("Error receiving request");
        sendMessage(r);
        endConnection();
        return;
    }
    auto h = msg.value().first;
    auto r = msg.value().second;

    printf("Request type: %d\n", r.type());

    if (r.type() == LOGIN) {
        handleLogin(r, h);
        return;
    } else if (r.type() == PING) {
        Response response;
        response.set_type(PONG);
        sendMessage(response);
        endConnection();
        return;
    } else if (r.type() == FRONTEND_SUBSCRIBE) {
        // Create Transaction
        auto transaction = new NewFrontendTransaction(r.frontend_subscription().hostname(), r.frontend_subscription().port());
        bool ok = server->getTransactionManager().doTransaction(*transaction);
        if (!ok) {
            Response r;
            r.set_type(ERROR);
            r.set_error_msg("Error creating frontend");
            sendMessage(r);
            endConnection();
            return;
        }
        Response r;
        r.set_type(LOGIN_OK);
        sendMessage(r);
        endConnection();
        return;
    }

    {
        // Check if session is valid
        std::string user;
        {
            auto read_guard = server->getReadStateGuard();
            printf("Session id: %ld\n", h.session_id);
            // Get the user
            auto &state = read_guard.get();
            if (!state.isSessionValid(h.session_id)) {
                Response r;
                r.set_type(ERROR);
                printf("Invalid session\n");
                r.set_error_msg("Invalid session");
                sendMessage(r);
                endConnection();
                return;
            }
            user = state.getUserFromSesssion(h.session_id);
        }

        // Run the handler
        if (r.type() == SUBSCRIBE) {
            handleSubscribe(r, user, h);
        } else if (r.type() == DOWNLOAD) {
            handleDownload(r, user, h);
        } else if (r.type() == LIST) {
            handleList(r, user, h);
        } else if (r.type() == LOGOUT) {
            handleLogout(r, user, h);
        } else if (r.type() == FILE_UPDATE) {
            handleFileUpdate(r, user, h);
        } else if (r.type() == UPLOAD) {
            handleUpload(r, user, h);
        } else {
            Response r;
            r.set_type(ERROR);
            r.set_error_msg("Unknown request type");
            printf("Unknown request type\n");
            sendMessage(r);
            endConnection();
            return;
        }

    }

}

void ServiceRequestHandler::handleLogin(Request request,
                                        Header header) {
    // Validate request
    if (request.username().empty() || request.password().empty()) {
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("Username or password empty");
        sendMessage(r);
        endConnection();
        return;
    }

    // Check State
    if (header.session_id > 0) {
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("Already logged in");
        sendMessage(r);
        endConnection();
        return;
    }

    // Check if user exists
    if (!server->getReadStateGuard().get().hasUser(request.username())) {
        // Create user
        CreateUserTransaction transaction(request.username(),
                                            request.password());
        bool ok = server->getTransactionManager().doTransaction(transaction);
        if (!ok) {
            Response r;
            r.set_type(ERROR);
            r.set_error_msg("Error creating user");
            sendMessage(r);
            endConnection();
            return;
        }
    }

    // Check password
    if (!server->getReadStateGuard().get().checkPassword(
            request.username(), request.password())) {
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("Wrong password");
        sendMessage(r);
        endConnection();
        return;
    }



    // Create session id
    int session_id;
    {
        auto guard  = server->getReadStateGuard();
        auto &state = guard.get();

        // Only two sessions per user
        if (state.getUserSessions(request.username()).size() >= 2) {
            Response r;
            r.set_type(ERROR);
            r.set_error_msg("Too many sessions");
            sendMessage(r);
            endConnection();
            return;
        }

        while (state.isSessionValid(session_id = rand()));
    }

    // Create Transaction
    auto transaction = new CreateSessionTransaction(session_id,
                                                    request.username());
    bool ok = server->getTransactionManager().doTransaction(*transaction);
    if (!ok) {
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("Error creating session");
        sendMessage(r);
        endConnection();
        return;
    }

    // Send response
    Response r;
    r.set_type(LOGIN_OK);
    r.set_session_id(session_id);
    sendMessage(r);
}

void ServiceRequestHandler::handleSubscribe(Request request,
                                            std::string user,
                                            Header header) {
    printf("Handling subscribe\n");
    // Get current tid
    int tid;
    {
        auto guard  = server->getReadStateGuard();
        auto &state = guard.get();
        tid = state.getLastTid();
    }

    // Wait for an update
    while (true) {
        auto guard  = server->getReadStateGuard();
        auto &state = guard.get();
        guard.wait([tid, &guard]() {
                return guard.get().getLastTid() > tid;
            });

        // Check if session is still valid
        if (!state.isSessionValid(header.session_id)) {
            endConnection();
            return;
        }
        // Now we have an update
        // TODO: Get the list of files that changed
        // TODO: Send the list of files
        printf("Sending update <-------------------------------------\n");

        for (auto &file: state.getUserFilesSince(user, tid)) {
            printf("Sending file update %s for user %s\n",
                   file->filename.c_str(), user.c_str());
            Response r;
            r.set_type(ResponseType::UPDATED);
            r.mutable_file_update()->set_filename(file->filename);
            r.mutable_file_update()->set_hash(file->hash);
            r.mutable_file_update()->set_deleted(file->deleted);
            bool ok = sendMessage(r);
            if (!ok) {
                printf("Error sending file update\n");
                return;
            }
        }

        tid = state.getLastTid();
    }
}

void ServiceRequestHandler::handleDownload(Request request,
                                           std::string user,
                                           Header header) {
    // Get the file
    auto guard  = server->getReadStateGuard();
    auto &state = guard.get();
    if (!state.hasFile(user, request.filename())) {
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("File not found");
        sendMessage(r);
        endConnection();
        return;
    }
    auto filepath = state.getFilePath(user, request.filename());

    printf("Sending file %s\n", filepath.c_str());
    std::fstream file(filepath, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("Error opening file");
        sendMessage(r);
        endConnection();
        return;
    }

    // Send the file
    Response r;
    r.set_type(ResponseType::FILE_DATA_R);
    r.mutable_file_data()->mutable_file()->set_filename(request.filename());
    r.mutable_file_data()->mutable_file()->set_hash(state.getFileHash(user, request.filename()));
    r.mutable_file_data()->mutable_data()->assign(std::istreambuf_iterator<char>(file),
                                                  std::istreambuf_iterator<char>());

    bool ok = sendMessage(r);
    if (!ok) {
        printf("Error sending file\n");
    }
    endConnection();
}

void ServiceRequestHandler::handleList(Request request,
                                       std::string user,
                                       Header header) {
    // Get the list of files
    auto guard  = server->getReadStateGuard();
    auto &state = guard.get();
    auto files  = state.getUserFiles(user);

    // Send the list of files
    Response r;
    r.set_type(ResponseType::FILE_LIST);
    for (auto &file: files) {
        auto fs = r.mutable_file_list()->add_files();
        fs->set_filename(file->filename);
        fs->set_hash(file->hash);
    }
    bool ok = sendMessage(r);
    if (!ok) {
        printf("Error sending file list\n");
        return;
    }
    endConnection();
}

void ServiceRequestHandler::handleLogout(Request request,
                                         std::string user,
                                         Header header) {
    // Create Transaction to remove session
    auto transaction = new RemoveSessionTransaction(header.session_id);
    bool ok = server->getTransactionManager().doTransaction(*transaction);
    if (!ok) {
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("Error removing session");
        sendMessage(r);
        endConnection();
        return;
    }
    else
    {
        Response r;
        r.set_type(LOGIN_OK);
        sendMessage(r);
        endConnection();
        return;
    }

}

void ServiceRequestHandler::handleFileUpdate(Request request,
                                             std::string user,
                                             Header header) {
    // Received the file update with the hash
    std::string filename = request.file_update().filename();
    std::string hash = request.file_update().hash();
    bool deleted = request.file_update().deleted();

    if (deleted) {
        printf("File deleted\n");
        // Delete the file
        byte_string empty = {0};
        auto transaction = new FileChangeTransaction(user, filename,
                                                     true, &empty);
        bool ok = server->getTransactionManager().doTransaction(
                *transaction);
        if (!ok) {
            Response r;
            r.set_type(ERROR);
            r.set_error_msg("Error deleting file");
            sendMessage(r);
            endConnection();
            return;
        } else {
            Response r;
            r.set_type(FILE_UPDATED);
            sendMessage(r);
            endConnection();
            return;
        }
    }
    {
        // Check if the file exists
        auto guard  = server->getReadStateGuard();
        auto &state = guard.get();
        printf("Received file update %s\n", filename.c_str());

        if (state.hasFile(user, filename)) {
            // Check if the hash is the same
            if (state.getFileHash(user, filename) == hash) {
                // Hash is the same, do nothing
                printf("Hash is the same, do nothing\n");
                Response r;
                r.set_type(FILE_UPDATED);
                sendMessage(r);
                endConnection();
                return;
            }
        }
    }

    // Ask the client to send the file
    printf("Asking client to send the file\n");
    Response r;
    r.set_type(SEND_FILE_DATA);
    sendMessage(r);

    // Receive the file
    auto msg = receiveRequest();
    if (!msg.has_value()) {
        printf("Error receiving request\n");
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("Error receiving request");
        sendMessage(r);
        endConnection();
        return;
    }
    auto h = msg.value().first;
    auto r2 = msg.value().second;
    if (r2.type() != FILE_DATA) {
        printf("Invalid request type\n");
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("Invalid request type");
        sendMessage(r);
        endConnection();
        return;
    }
    handleUpload(r2, user, h);
}

void ServiceRequestHandler::handleUpload(Request request,
                                         std::string user,
                                         Header header) {
    byte_string data =
            (std::basic_string<unsigned char> &) request
                    .file_data_update()
                    .data();
    // Create a transaction to update the file
    auto transaction = new FileChangeTransaction(user,
                                                 request.file_data_update().file_update().filename(),
                                                 false,
                                                    &data);
    bool ok = server->getTransactionManager().doTransaction(*transaction);
    if (!ok) {
        Response r;
        r.set_type(ERROR);
        r.set_error_msg("Error updating file");
        sendMessage(r);
        endConnection();
        return;
    }
    else
    {
        Response r;
        r.set_type(FILE_UPDATED);
        sendMessage(r);
        endConnection();
        return;
    }
}
