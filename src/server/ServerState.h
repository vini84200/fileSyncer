//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_SERVERSTATE_H
#define FILESYNCERCLIENT_SERVERSTATE_H

#include "service/ServerRequestHandler.h"
#include <map>
#include <string>
#include <vector>

typedef int SessionId;

class ServerState {
    std::map<std::string, std::string> senhas;
    std::map<SessionId, std::string> logged_user_sessions;
    std::map<std::string,
             std::vector<std::tuple<int, ServerRequestHandler *>>>
            subscribed_users;


public:
    void addSession(SessionId session_id, std::string username);
    void removeSession(SessionId session_id);
    bool isSessionValid(SessionId session_id);

    int getNumberOfSessions();
    std::string getUsernameFromSession(SessionId session_id);

    bool isValid();
};


#endif//FILESYNCERCLIENT_SERVERSTATE_H
