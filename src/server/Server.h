//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_SERVER_H
#define FILESYNCERCLIENT_SERVER_H

#include "../common/RwLock.h"
#include "ServerState.h"
#include "TransactionManager.h"
#include "service/ServiceListener.h"

class AdminListener;

class Server {
public:
    Server();

    void start();
    void stop();

    ReadLock<ServerState> getReadStateGuard();
    WriteLock<ServerState> getWriteStateGuard();

    TransactionManager &getTransactionManager();

private:
    SharedData<ServerState> state;
    bool isCoordinator;
    bool isRunning;

    void loadConfig();
    int server_id{};
    int server_port{};
    int server_admin_port{};
    std::string server_host;
    std::vector<std::tuple<int, std::string>> servers;

    TransactionManager transaction_manager;
    AdminListener *admin_listener;
    ServiceListener *service_listener;

    void startCoordinator();
    void startElection();

    void startAdminListener();
    unsigned int warmup_time = 15;
    int coordinator_id       = -1;
    bool checkCoordinatorAlive();
    void startTransactionListener();
};


#endif//FILESYNCERCLIENT_SERVER_H
