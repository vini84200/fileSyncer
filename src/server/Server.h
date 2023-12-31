//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_SERVER_H
#define FILESYNCERCLIENT_SERVER_H

#include "../common/RwLock.h"
#include "Replica.h"
#include "Election.h"
#include "ServerState.h"
#include "service/ServiceListener.h"
#include "transactions/TransactionManager.h"

class AdminListener;
class TransactionListener;

class Server {
public:
    Server();

    void start();
    void stop();

    ReadLock<ServerState> getReadStateGuard();
    WriteLock<ServerState> getWriteStateGuard();

    TransactionManager &getTransactionManager();

    std::vector<Replica *> getActiveServers();
    bool hasCoordinator();
    void setCoordinator(int id);
    Replica& getCoordinator();
    void startElection();
    std::vector<Replica *> getReplicas();

    int getId();

    void startCoordinator();

    bool hasElection();

    Replica &getReplica(int i);

    Election &getElection();

private:
    SharedData<ServerState> state;



    bool isCoordinator;
    bool isRunning;

    void loadConfig();
    int server_id{};
    int server_port{};
    int server_admin_port{};
    int server_transaction_port{};
    std::string server_host;
    std::map<int, Replica> servers;

    TransactionManager transaction_manager;
    AdminListener *admin_listener = nullptr;
    ServiceListener *service_listener = nullptr;
    TransactionListener *transaction_listener = nullptr;


    void startAdminListener();
    unsigned int warmup_time = 15;
    int coordinator_id       = -1;
    bool checkCoordinatorAlive();
    void startTransactionListener();


    std::string getSyncerDirFromConfig();
    Election *election = nullptr;
};


#endif//FILESYNCERCLIENT_SERVER_H
