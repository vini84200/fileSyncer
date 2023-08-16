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
