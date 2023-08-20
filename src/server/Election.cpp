#include "Election.h"

#include "Server.h"
#include "Replica.h"

using namespace std::chrono_literals;

void Election::startElection() {
    int serverId = server->getId();
    std::vector<Replica *> servers = getServers();
    std::cout << "Server " << serverId << " is starting the election.\n";
    highestAliveReply = serverId;

    // Find the highest id
    int hi = serverId;
    for (auto &server : servers) {
        if (server->getId() > hi) {
            hi = server->getId();
        }
    }
    if (hi == serverId) {
        // This server is the highest id
        // This server becomes the leader
        std::cout << "Server " << serverId << " becomes the leader.\n";
        winElection();
        return;
    }
    // Send election message to higher server ids
    for (auto &server : getMoreImportantServers()) {
        std::cout << "Server " << serverId << " sends ELECTION message to server " << server->getId() << ".\n";
        informElection(server);
    }

    // Waits timeout or reply from higher server ids
    waitReply();
    printf("The server %d has received a reply from server %d\n", serverId, highestAliveReply);

    // Receive responses
    // Check if there is a higher id


    if (highestAliveReply == serverId) {
        // This server becomes the leader
        std::cout << "Server " << serverId << " becomes the leader.\n";
        winElection();
    } else {
        // Wait for the coordinator message
        std::cout << "Server " << serverId << " waits for the coordinator message.\n";
        waitVictory();
        if (victorious == -1) {
            // No coordinator found in the timeout
            // Start a new election
            startElection();
        }
        else {
            // There is a coordinator
            // Set the coordinator
            server->setCoordinator(victorious);
        }
    }
}

std::vector<Replica *> Election::getServers() {
    return server->getReplicas();
}
Election::Election(Server *server)
        : server(server) {
}

void Election::waitReply() {
    std::unique_lock<std::mutex> lock(m);
    cv.wait_for(lock, 5000ms, [this] { return highestAliveReply  > server->getId(); });
    // We have a reply or the timeout has expired

}

void Election::waitVictory() {
    std::unique_lock<std::mutex> lock(m);
    cv.wait_for(lock, 5000ms, [this] { return victorious != -1; });
    // We have a reply or the timeout has expired

}

void Election::winElection() {
    victorious = server->getId();
    server->setCoordinator(victorious);
    // Send coordinator message to all servers
    for (auto &server : getServers()) {
        informVictory(server);
    }
    server->startCoordinator();

}

void Election::informVictory(Replica *&pReplica) {
    Connection<AdminMsg, AdminMsg> connection(pReplica->getAdminConnectionArgs());
    AdminMsg msg;
    msg.set_type(AdminMsgType::ELECTION_COORDINATOR);
    msg.set_sender_id(server->getId());
    connection.sendMessage(msg);
    // Do not care about the response
}

void Election::informElection(Replica *pReplica) {
    // Send election message
    Connection<AdminMsg, AdminMsg> connection(pReplica->getAdminConnectionArgs());
    AdminMsg msg;
    msg.set_type(AdminMsgType::ELECTION);
    msg.set_sender_id(server->getId());
    connection.sendMessage(msg);

    // The response should come in another connection
}

void Election::receiveAnswer(int32_t i) {
    std::unique_lock<std::mutex> lock(m);
    if (i > highestAliveReply) {
        highestAliveReply = i;
    }
}

void Election::receiveCoordinatorMessage(int id) {
    std::unique_lock<std::mutex> lock(m);
    if (victorious == -1) {
        victorious = id;
    }
}

std::vector<Replica *> Election::getMoreImportantServers() {
    std::vector<Replica *> servers = getServers();
    std::vector<Replica *> moreImportantServers;
    for (auto &server : servers) {
        if (server->getId() > this->server->getId()) {
            moreImportantServers.push_back(server);
        }
    }
    return moreImportantServers;
}
