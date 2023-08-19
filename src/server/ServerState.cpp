//
// Created by vini84200 on 16/08/23.
//

#include "ServerState.h"

#include <filesystem>
#include <fstream>
#include <utility>

// GLOBAL

std::string ServerState::serverDir = ".syncer/";

// CONSTRUCTORS

ServerState::ServerState() {
    // Get file list from disk
    init();
}

void ServerState::init() {
    std::string servers_dir = getServersDir();
    std::filesystem::create_directory(servers_dir);

    for (auto const &dir_entry:
         std::filesystem::directory_iterator(servers_dir)) {

        std::string username = dir_entry.path().filename();
        std::string user_dir = dir_entry.path();
        // Remove if it's a temporary directory
        if (username.back() == '~') {
            std::filesystem::remove(user_dir);
            continue;
        }
        if (std::filesystem::is_directory(user_dir)) {
            for (auto const &file_entry:
                 std::filesystem::directory_iterator(user_dir)) {
                std::string filename  = file_entry.path().filename();
                std::string file_path = file_entry.path();
                // Ignore if it's a directory
                if (std::filesystem::is_directory(file_path)) {
                    printf("WARN: Found directory in user directory "
                           "%s\n",
                           file_path.c_str());
                    continue;
                }
                // Delete if it is a temp file, finished with ~
                if (filename.back() == '~') {
                    std::filesystem::remove(file_path);
                    continue;
                }
                // If it's the password file, get the password
                if (filename == ".password") {
                    std::ifstream password_file(file_path);
                    std::string password;
                    password_file >> password;
                    printf("Found password for user %s: ****\n",
                           username.c_str());
                    this->senhas.emplace(username, password);
                    continue;
                }

                std::string digest =
                        digest_to_string(getFileDigest(file_path));
                printf("Found file for user %s: %s (%s)\n",
                       username.c_str(), filename.c_str(),
                       digest.c_str());
                this->user_files[username].emplace(filename, digest);
            }
            continue;
        }
        if (username == "tid") {
            std::ifstream tid_file(user_dir);
            tid_file >> this->last_tid;
            printf("Found tid file, last tid is %d\n", this->last_tid);
            continue;
        }
    }
}

// GETTERS

bool ServerState::isValid() const {
    return true;
}

bool ServerState::hasUser(const std::string &username) const {
    return senhas.find(username) != senhas.end();
}

bool ServerState::checkPassword(const std::string &username,
                                const std::string &password) const {
    return senhas.at(username) == password;
}

bool ServerState::isSessionValid(SessionId session_id) const {
    return logged_user_sessions.count(session_id) != 0;
}

std::string ServerState::getUserFromSesssion(long session) const {
    return logged_user_sessions.at(session);
}

int ServerState::getLastTid() const {
    return last_tid;
}

int ServerState::getUserLastTid(std::string username) const {
    return users_last_tid.count(username) == 0
                   ? 0
                   : users_last_tid.at(username);
}

// MUTATIONS

void ServerState::addSession(SessionId session_id,
                             std::string username) {
    logged_user_sessions.emplace(session_id, std::move(username));
}

void ServerState::addUser(std::string username,
                          std::string password) {
    senhas.emplace(std::move(username), std::move(password));
}

void ServerState::removeSession(SessionId session_id) {
    logged_user_sessions.erase(session_id);
}

void ServerState::setUserLastTid(std::string username, int tid) {
    users_last_tid.emplace(std::move(username), tid);
}

void ServerState::setLastTid(int lastTid) {
    last_tid = lastTid;
}

std::string ServerState::getServersDir() {
    std::string home = getenv("HOME");
    return home + "/" + serverDir;
}

std::string ServerState::getFilePath(std::string username,
                                     std::string filename) {
    return getUserDir(username) + filename;
}

std::string ServerState::getUserDir(std::string user) {
    return getServersDir() + user;
}

std::string ServerState::getTidPath() {
    return getServersDir() + "tid";
}

ServerState::ServerState(std::string serverDir) {
    ServerState::serverDir = std::move(serverDir);
    init();
}
