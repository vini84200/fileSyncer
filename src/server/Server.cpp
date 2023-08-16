//
// Created by vini84200 on 16/08/23.
//

#include "Server.h"
#include "admin/AdminListener.h"
#include <fstream>
#include <sstream>

Server::Server() {
    isCoordinator = false;
    state_        = new ServerState();

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
    * server_id: 1
    * server_port: 8989
    * server_admin_port: 8998
    * server_host: 0.0.0.0
    * warmup_time: 5
    * other_servers_ip:
    * - 231.122.233.11  4 // IP and id of the other servers
    * - 231.122.233.14  7
    * */

    std::string line;
    while (std::getline(config_file, line)) {
        std::string key, value;
        std::istringstream iss(line);
        iss >> key >> value;
        if (key == "server_id:") {
            server_id = std::stoi(value);
        } else if (key == "server_port:") {
            server_port = std::stoi(value);
        } else if (key == "server_admin_port:") {
            server_admin_port = std::stoi(value);
        } else if (key == "server_host:") {
            server_host = value;
        } else if (key == "warmup_time:") {
            warmup_time = std::stoi(value);
        } else if (key == "other_servers_ip:") {
            while (std::getline(config_file, line)) {
                // Remove the dash
                line = line.substr(2);
                std::istringstream iss(line);
                std::string ip;
                int id;

                iss >> ip >> id;
                servers.emplace_back(id, ip);
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
    sleep(warmup_time);
    // Check if there is a coordinator
    if (coordinator_id == -1) {
        startElection();
    } else {
        // Check if the coordinator is alive
        // If it is not, start the election
        if (!checkCoordinatorAlive()) { startElection(); }
    }

    while (isRunning) {
        sleep(2);
    }
}

void Server::startAdminListener() {
    admin_listener =
            new AdminListener(server_host, server_admin_port, this);
    admin_listener->start();
}

void Server::startElection() {
    // TODO: Implement
}

void Server::startCoordinator() {
    // TODO: Implement
}

bool Server::checkCoordinatorAlive() {
    return false;// TODO: Implement
}

void Server::startTransactionListener() {
    // TODO: Implement
}

void Server::stop() {
    isRunning = false;
    if (admin_listener != nullptr) { admin_listener->stop(); }
    // TODO: Stop the other listeners
}
