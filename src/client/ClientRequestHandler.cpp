#include "../server/interfaces/RequestHandler.h"
#include <string>
#include <iostream>
#include <unistd.h>

class ClientRequestHandler : public RequestHandler<std::string> {
    public:
        ClientRequestHandler(int socket) : RequestHandler(socket) {}

    protected:
        void handleRequest() override{
            char buff[256];
            ssize_t bytes_read = read(client_fd, buff, sizeof(buff) -1 );
            if(bytes_read <= 0) {
                return;
            }
            buff[bytes_read] = '\0';
            std::string message(buff);

            if (message == "change_IP"){
                std::cout << "Change IP Request Received" << std::endl;
            }
        }
};
