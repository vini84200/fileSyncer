//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_SERVERSTATE_H
#define FILESYNCERCLIENT_SERVERSTATE_H

#include "../common/utils.h"
#include <map>
#include <string>
#include <vector>

typedef int SessionId;

struct UserFile {
    std::string filename;
    std::string hash;
    int last_tid;
    int size;
    bool deleted;
};

typedef std::map<std::string, UserFile> UserFileList;

class ServerState {
public:
    ServerState();
    ServerState(std::string serverDir);

private:
    std::map<std::string, std::string> senhas{};
    std::map<SessionId, std::string> logged_user_sessions{};

    // Last transaction id for each user that affected the server state
    std::map<std::string, int> users_last_tid{};
    int last_tid = 0;
    std::map<std::string, UserFileList> user_files{};

public:
    int getLastTid() const;
    void setLastTid(int lastTid);

    void addSession(SessionId session_id, std::string username);
    void removeSession(SessionId session_id);
    bool isSessionValid(SessionId session_id) const;

    int getNumberOfSessions();
    bool isValid() const;
    [[nodiscard]] bool hasUser(const std::string &username) const;
    bool checkPassword(const std::string &username,
                       const std::string &password) const;
    void addUser(std::string username, std::string password);
    std::string getUserFromSesssion(long session) const;
    void setUserLastTid(std::string username, int tid);
    int getUserLastTid(std::string username) const;

    static std::string serverDir;
    static std::string getServersDir();
    static std::string getFilePath(std::string username,
                                   std::string filename);
    static std::string getUserDir(std::string user);
    static std::string getTidPath();


    void init();
    std::vector<const UserFile *> getUserFilesSince(std::string user,
                                                    int tid) const;
    std::vector<const UserFile *>
    getUserFiles(std::string user) const;
    bool hasFile(std::string username, std::string filename) const;
    const std::string getFileHash(std::string user,
                                   const std::string &filename) const;
    void removeFile(std::string user, std::string filename);
    void updateFile(std::string user, std::string filename,
                    UserFile file);
    void addFile(std::string user, std::string filename,
                 UserFile file);
};


#endif//FILESYNCERCLIENT_SERVERSTATE_H
