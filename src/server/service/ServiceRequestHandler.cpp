//
// Created by vini84200 on 8/16/23.
//

#include "ServiceRequestHandler.h"
#include "../Server.h"
#include "../transactions/CreateSessionTransaction.h"
#include "../transactions/CreateUserTransaction.h"

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
    }

    {
        // Check if session is valid
        std::string user;
        {
            auto read_guard = server->getReadStateGuard();
            // Get the user
            auto &state = read_guard.get();
            if (!state.isSessionValid(h.session_id)) {
                Response r;
                r.set_type(ERROR);
                r.set_error_msg("Invalid session");
                sendMessage(r);
                endConnection();
                return;
            }
            user = state.getUserFromSesssion(h.session_id);
        }

        // Run the handler

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
