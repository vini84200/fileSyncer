#ifndef FILESYNCERCLIENT_ELECTION_H
#define FILESYNCERCLIENT_ELECTION_H

#include <string>
#include <iostream>
#include <vector>

#include <condition_variable>

class Server;
class Replica;

/**
 * Election class
 * Implements the Bully algorithm
 * https://en.wikipedia.org/wiki/Bully_algorithm
 */
class Election {

private:
    Server* server;

    int highestAliveReply = -1;

public:
    Election(Server* server);

    std::vector<Replica *> getServers();
    std::vector<Replica *> getMoreImportantServers();


    void informElection(Replica *pReplica);

    void receiveCoordinatorMessage(int id);

    void startElection();
    void waitReply();
    void waitVictory();
    int victorious = -1;
    void winElection();
    void informVictory(Replica *&pReplica);
    void receiveAnswer(int32_t i);
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    std::mutex m;
    std::condition_variable cv;
};


#endif//FILESYNCERCLIENT_ELECTION_H