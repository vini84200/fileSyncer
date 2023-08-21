//
// Created by vini84200 on 8/18/23.
//

#include "Replica.h"
#include "Server.h"

bool Replica::checkAliveBlocking() {
    Connection<AdminMsg, AdminMsg> connection(getAdminConnectionArgs());
    if (!isAlive) return false;
    AdminMsg* hbReq  = new AdminMsg();
    hbReq->set_type(AdminMsgType::HEARTBEAT);
    connection.sendRequest(*hbReq);
    connection.setTimout(HEARTBEAT_TIMEOUT);
    auto hbResp =
            connection.receiveResponse();
    if (!hbResp.has_value()) {
        printf("Replica %d is dead!!\n", id);
        pthread_mutex_lock(&mutex);
        isAlive = false;
        pthread_mutex_unlock(&mutex);
        return false;
    }
    auto header = hbResp.value().first;
    auto hbResp1 = hbResp.value().second;
    if (hbResp1.type() == AdminMsgType::HEARTBEAT) {
        pthread_mutex_lock(&mutex);
        if (!isAlive) {
            printf("Replica %d is alive again!! It will be ignored because it has an old state\n", id);
        } else {
            isAlive = true;
            // Save the last heartbeat time
            lastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
            ).count();
        }
        pthread_mutex_unlock(&mutex);
        return true;
    }
    else {
        // What!?
        printf("Replica %d sent an invalid response to the heartbeat\n", id);
        printf("Response type: %d\n", hbResp1.type());
    }

}

bool Replica::checkAlive() {
    pthread_t thread;
    pthread_create(&thread, NULL, checkAliveRunThread, this);
}

void *Replica::checkAliveRunThread(void *replica) {
    Replica* replica1 = (Replica*) replica;
    replica1->checkAliveBlocking();
}
