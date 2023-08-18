#ifndef FILESYNCERCLIENT_ELECTION_H
#define FILESYNCERCLIENT_ELECTION_H

#include <string>
#include <iostream>
#include <vector>

class Election {
private:
    int serverId;
    std::vector<int> servers;


public:
    Election(int id, const std::vector<int>& ids) : serverId(id), servers(ids) {}

    void startElection();
};


#endif//FILESYNCERCLIENT_ELECTION_H