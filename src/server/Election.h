#ifndef FILESYNCERCLIENT_ELECTION_H
#define FILESYNCERCLIENT_ELECTION_H

#include <string>
#include <iostream>
#include <vector>
#include "Server.h"

class Election {
private:
    int serverId;
    std::vector<Replica*> servers;


public:
    Election(int id, const std::vector<Replica*>& ids) : serverId(id), servers(ids) {}

    void startElection();
};


#endif//FILESYNCERCLIENT_ELECTION_H