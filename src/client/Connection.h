#ifndef FILESYNCER_CONNECTION_H
#define FILESYNCER_CONNECTION_H

#include <cstddef>

#define CONNECTION_WAS_CLOSED 0
#define SEND_ERROR -1


#include <optional>
#include <string>
#include "../common/MessageComunication.h"

struct ConnectionArgs {
    std::string hostname;
    int port;

    std::string username;
    std::string password;

    ConnectionArgs(
            std::string hostname,
            int port,
            std::string username,
            std::string password
    ) : hostname(hostname), port(port), username(username), password(password) {};
};


class Connection {

public:
    Connection(ConnectionArgs args);
    Connection(Connection &other); // copy constructor
    ~Connection();

    enum class ConnectionState {
        NOT_INITIALIZED,
        CONNECTED,
        CONNECTION_CLOSED,
        CONNECTION_FAILURE
    };

    ConnectionState getConnectionState();

public:
    std::optional<std::pair<Header, std::vector<char>>> receiveMsg();

    bool receiveBytes(char *bytes, size_t bytes_to_receive);

    bool sendMessage(Message msg);

    int doLogin(std::string username, std::string password);


private:
    ConnectionState currConnState = ConnectionState::NOT_INITIALIZED;
    int connectionFD = 0;
    long int sesstionId = 0;
    ConnectionArgs args;
};

#endif
