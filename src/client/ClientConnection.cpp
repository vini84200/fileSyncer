//
// Created by vini84200 on 8/17/23.
//

#include "ClientConnection.h"

int ClientConnection::doLogin(std::string username, std::string password) {
    sesstionId = -1;
    if (currConnState != ConnectionState::CONNECTED) {
        return -1;
    }
    Request req;
    req.New();
    req.set_type(LOGIN);
    req.set_username(username);
    req.set_password(password);

    Message m(req);
    Connection loginConn(*this);
    if (loginConn.getConnectionState() != ConnectionState::CONNECTED) {
        perror("Login failed");
        return -1;
    }
    loginConn.sendMessage(m);

    auto msg = loginConn.receiveMsg();
    if (!msg.has_value()) {
        return -1;
    } else {
        auto [header, response] = msg.value();
        Response resp;
        resp.ParseFromArray(response.data(), response.size());

        if (resp.type() == LOGIN_OK) {
            // Extract token
            sesstionId = resp.session_id();
            return 0;
        } else {
            return -1;
        }
    }


}

bool ClientConnection::isLogged() {
    return sesstionId != -1;
}