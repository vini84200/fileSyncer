#include "Election.h"

void Election::startElection() {
    std::cout << "Server " << serverId << " is starting the election.\n";
    int highestId = serverId;

    // Send election message to higher server ids
    for (int i = serverId + 1; i < servers.size(); ++i) {
        std::cout << "Server " << serverId << " sends ELECTION message to server " << servers[i] << ".\n";
    }

    // Receive responses
    for (int i = serverId + 1; i < servers.size(); ++i) {
        // Simulating a response from servers with higher IDs
        if (servers[i] > highestId) {
            highestId = servers[i];
        }
    }

    if (highestId == serverId) {
        // This server becomes the leader
        std::cout << "Server " << serverId << " becomes the leader.\n";
    } else {
        // Send COORDINATOR message to the elected leader
        std::cout << "Server " << serverId << " sends COORDINATOR message to servers " << highestId << ".\n";
    }
}
