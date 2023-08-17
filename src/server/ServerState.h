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

    int last_tid = 0;

public:
    void addSession(SessionId session_id, std::string username);
    void removeSession(SessionId session_id);
    bool isSessionValid(SessionId session_id)const;

    int getNumberOfSessions();
    bool isValid();
    [[nodiscard]] bool hasUser(const std::string &username) const;
    int nextTid() const;
    bool checkPassword(const std::string &username,
                       const std::string &password) const;
    void addUser(std::string username, std::string password);
    std::string getUserFromSesssion(long session) const;
};


#endif//FILESYNCERCLIENT_SERVERSTATE_H
