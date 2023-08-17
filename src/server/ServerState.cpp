//
// Created by vini84200 on 16/08/23.
//

#include "ServerState.h"

#include <utility>

void ServerState::addSession(SessionId session_id,
                             std::string username) {
    logged_user_sessions[session_id] = std::move(username);
}

bool ServerState::isValid() {
    return true;
}

bool ServerState::hasUser(const std::string &username) const {
    return senhas.find(username) != senhas.end();
}

int ServerState::nextTid() const {
    return last_tid + 1;
}

bool ServerState::checkPassword(const std::string &username,
                                const std::string &password) const {
    return senhas.at(username) == password;
}

bool ServerState::isSessionValid(SessionId session_id) const {
    return logged_user_sessions.find(session_id) != logged_user_sessions.end();
}

void ServerState::addUser(std::string username,
                          std::string password) {
    senhas.emplace(std::move(username), std::move(password));
}
