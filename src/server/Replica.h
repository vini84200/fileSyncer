//
// Created by vini84200 on 8/18/23.
//

#ifndef FILESYNCERCLIENT_REPLICA_H
#define FILESYNCERCLIENT_REPLICA_H

#include "../common/Connection.h"
#include <string>

#define HEARTBEAT_TIMEOUT 1400

class Replica {
private:
    int id;
    std::string host;
    int adminPort;
    int servicePort;
    int transactionPort;
    bool isCoordinator = false;
    bool isAlive = true;
    int lastTid;
    long lastHeartbeat;

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
public:
    Replica(int id, std::string host, int adminPort, int servicePort, int transactionPort)
        : id(id), host(std::move(host)), adminPort(adminPort), servicePort(servicePort), transactionPort(transactionPort) {
        isCoordinator = false;
    }

    Replica(int id, std::string host, int adminPort, int servicePort, int transactionPort, bool isCoordinator)
        : id(id), host(std::move(host)), adminPort(adminPort), servicePort(servicePort), transactionPort(transactionPort), isCoordinator(isCoordinator) {
    }

    bool checkAliveBlocking();
    bool checkAlive();
    static void* checkAliveRunThread(void* replica);

    ConnectionArgs getServiceConnectionArgs() {
        return ConnectionArgs(host, servicePort);
    }
    ConnectionArgs getAdminConnectionArgs() {
        return ConnectionArgs(host, adminPort);
    }
    ConnectionArgs getTransactionConnectionArgs() {
        return ConnectionArgs(host, transactionPort);
    }


    // Getters and Setters

    int getId() const { return id; }

    void setId(int id) { Replica::id = id; }

    const std::string &getHost() const { return host; }

    void setHost(const std::string &host) { Replica::host = host; }

    int getAdminPort() const { return adminPort; }

    void setAdminPort(int adminPort) {
        Replica::adminPort = adminPort;
    }

    int getServicePort() const { return servicePort; }

    void setServicePort(int servicePort) {
        Replica::servicePort = servicePort;
    }

    int getTransactionPort() const { return transactionPort; }

    void setTransactionPort(int transactionPort) {
        Replica::transactionPort = transactionPort;
    }

    bool isCoordinator1() const { return isCoordinator; }

    void setIsCoordinator(bool isCoordinator) {
        Replica::isCoordinator = isCoordinator;
    }

    bool isAlive1() const { return isAlive; }

    void setIsAlive(bool isAlive) { Replica::isAlive = isAlive; }

    int getLastTid() const { return lastTid; }

    void setLastTid(int lastTid) { Replica::lastTid = lastTid; }
};


#endif//FILESYNCERCLIENT_REPLICA_H
