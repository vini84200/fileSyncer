#ifndef CLIENTLISTENER_H
#define CLIENTLISTENER_H

#include "../server/interfaces/Listener.h"
#include "ClientRequestHandler.h"

class ClientListener : public Listener<std::string> {
    public:
        ClientListener(const std::string& host, int port);

    protected:
        RequestHandler<std::string>* createRequestHandler(int socket) override;
        std::string getListenerName() override;
    private:
};

#endif // CLIENTLISTENER_H
