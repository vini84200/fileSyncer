//
// Created by vini84200 on 16/08/23.
//

#include "Server.h"
#include "../common/RwLock.h"
#include "admin/AdminListener.h"
#include "transactions/TransactionListener.h"
#include <fstream>
#include <sstream>

Server::Server()
    : state(ServerState()), transaction_manager(false, this, state) {
    isCoordinator             = false;
    ServerState initial_state = ServerState();


    loadConfig();
}

void Server::loadConfig() {
    // Open the config file
    std::ifstream config_file;
    config_file.open("config.txt");
    if (!config_file.is_open()) {
        printf("ERROR: Could not open config file\n");
        exit(1);
    }

    // Read the config file

    /*Config example:
    server_id: 1
    server_port: 8989
    server_admin_port: 8998
    server_transaction_port: 8999
    server_host: 0.0.0.0
    warmup_time: 5
    Comments are allowed
    other_servers_ip:
    - 3 231.122.233.11 1002 1003 1004 # Id IP ServicePort AdminPort TransactionPort
    * */

    std::string line;
    while (std::getline(config_file, line)) {
        // Ignore comments
        if (line[0] == '#') { continue; }

        std::string key, value;
        std::istringstream iss(line);
        iss >> key >> value;
        if (key == "server_id:") {
            server_id = std::stoi(value);
        } else if (key == "server_port:") {
            server_port = std::stoi(value);
        } else if (key == "server_admin_port:") {
            server_admin_port = std::stoi(value);
        } else if (key == "server_transaction_port:") {
            server_transaction_port = std::stoi(value);
        } else if (key == "server_host:") {
            server_host = value;
        } else if (key == "warmup_time:") {
            warmup_time = std::stoi(value);
        } else if (key == "other_servers_ip:") {
            while (std::getline(config_file, line)) {
                // Ignore comments
                if (line[0] == '#') { continue; }
                // Ignore empty lines
                if (line.empty()) { continue; }
                // Ignore comments at the end of the line
                if (line.find('#') != std::string::npos) {
                    line = line.substr(0, line.find('#'));
                }
                // Remove the dash
                line = line.substr(2);
                std::istringstream iss(line);
                int id;
                std::string ip;
                int service_port;
                int admin_port;
                int transaction_port;
                iss >> id >> ip >> service_port >> admin_port >>
                    transaction_port;
                servers.emplace(id, Replica(id, ip, admin_port, service_port,
                                            transaction_port));
            }
        }
    }
}

void Server::start() {
    isRunning = true;
    startAdminListener();
    startTransactionListener();
    // Waits warmup time to start the election
    // This is to prevent the servers from starting the election before
    // all of them are up and listening to the admin port
    printf("Waiting %d seconds to start the election\n", warmup_time);
    sleep(warmup_time);
    printf("Warmup time finished\n");
    // Check if there is a coordinator
    if (coordinator_id == -1) {
        printf("No coordinator found\n Starting an election\n");
        startElection();
    } else {
        // Check if the coordinator is alive
        // If it is not, start the election
        printf("Coordinator found\n");
        if (!checkCoordinatorAlive()) { startElection(); }
    }

    while (isRunning) { sleep(2); }
}

void Server::startAdminListener() {
    admin_listener =
            new AdminListener(server_host, server_admin_port, this);
    admin_listener->start();
}

void Server::startElection() {
    // TODO: Implement the election algorithm
    // Temporary solution: The server with ID 1 is the coordinator
    setCoordinator(1);
}

void Server::startCoordinator() {
    printf("We are the coordinator\n");
    transaction_manager.setIsCoordinator(true);

    // TODO: Notify the other servers

    // Start the recovery process

    // Check all the servers that are up
    for (auto &server: servers) {
        int id = std::get<0>(server);
        // If the server is not the coordinator and is up
        if (id != coordinator_id && id != server_id) {
            // Send the heartbeat request
            // If the server is down, remove it from the list
            // If the server is up, add it to the list

            // TODO: Implement the heartbeat request
            if (true) {
                for (auto &[id, server]: servers) {
                    server.checkAliveBlocking();
                }
            }
        }
    }

    // Start the Service Listener
    service_listener =
            new ServiceListener(server_host, server_port, this);
    service_listener->start();
}

bool Server::checkCoordinatorAlive() {
    return false;// TODO: Implement
}

void Server::startTransactionListener() {
    transaction_listener =
            new TransactionListener(server_host, server_transaction_port,
                                    this);
    transaction_listener->start();
}

void Server::stop() {
    isRunning = false;
    if (admin_listener != nullptr) { admin_listener->stop(); }
    // TODO: Stop the other listeners
}

ReadLock<ServerState> Server::getReadStateGuard() {
    return {state};
}

WriteLock<ServerState> Server::getWriteStateGuard() {
    return {state};
}

TransactionManager &Server::getTransactionManager() {
    return transaction_manager;
}

std::vector<Replica*> Server::getActiveServers() {
    std::vector<Replica*> active_servers;
    for (auto &[id, server]: servers) {
        if (server.isAlive1()) {
            active_servers.push_back(&server);
        }
    }
    return active_servers;
}

Replica &Server::getCoordinator() {
    return servers.at(coordinator_id);
}

void Server::setCoordinator(int id) {
    coordinator_id = id;
    isCoordinator  = id == server_id;
    printf("Coordinator set to %d\n", id);
    if (isCoordinator) {
        startCoordinator();
    }

}
