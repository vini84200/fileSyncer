#include "Connection.h"
#include <array>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <vector>

Connection::Connection (ConnectionArgs args) {
  currConnState = ConnectionState::NOT_INITIALIZED;
  struct addrinfo hints, *servinfo, *p;
  int rv;


  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC; 
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(args.hostname.c_str(), std::to_string(args.port).c_str(), &hints, &servinfo)) != 0) {
    currConnState = ConnectionState::CONNECTION_FAILURE;
    return;
  }

  for (p = servinfo; p!=nullptr; p = p->ai_next) {
    if ((connectionFD  = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      currConnState = ConnectionState::CONNECTION_FAILURE;
      continue;
    }

    if (connect(connectionFD, p->ai_addr, p->ai_addrlen) == -1) {
      currConnState = ConnectionState::CONNECTION_FAILURE;
      close(connectionFD);
      continue;
    }
    break;
  }

  if (p == NULL) {
    currConnState = ConnectionState::CONNECTION_FAILURE;
  } else {
    currConnState = ConnectionState::CONNECTED;
  }

  freeaddrinfo(servinfo);

  this->doLogin(args.username, args.password);

} 

Connection::~Connection() {
  if (currConnState == ConnectionState::CONNECTED) {
    close(connectionFD);
  }
}

Connection::ConnectionState Connection::getConnectionState() {
 return currConnState;
}

std::optional<std::pair<Header, std::vector<char>>> Connection::receiveMsg() {
  std::array<char, Header::getHeaderSize()> header_buff;
  bool ok = receiveBytes(header_buff.data(), Header::getHeaderSize());
  if (!ok) {
    return {};
  }

  Header h;
  if ( h.loadFromBuffer(header_buff.data()) == -1 ) {
    return {};
  }

  std::vector<char> msg_buff;
  msg_buff.assign(0, h.msg_size);

  if (!receiveBytes(msg_buff.data(), h.msg_size)) {
    return {};
  }
  
  return {{h, msg_buff}};
}


bool Connection::receiveBytes(char* bytes, size_t bytes_to_receive) {
  int received = 0;
  int now = 0;
  while (received < bytes_to_receive) {
    now = recv(connectionFD, bytes, bytes_to_receive, 0);
    if (now == CONNECTION_WAS_CLOSED) {
      currConnState = ConnectionState::CONNECTION_CLOSED;
      return false;
    }
    if (now ==SEND_ERROR) {
      currConnState = ConnectionState::CONNECTION_FAILURE;
      return false;
    }
    received += now;
  }
  return true;
}

int Connection::doLogin(std::string username, std::string password) {


}

bool Connection::sendMessage(Message msg) {
  if (currConnState != ConnectionState::CONNECTED) {
    return false;
  }
  SealedMessage sm (msg);
  void  *buffer = sm.getSealedMessagePtr();
  int hasToSend = sm.getSealedMessageSize();
  int sent = 0;

  while (sent < hasToSend) {
    int sent_now = send(connectionFD, buffer, sm.getSealedMessageSize(), 0);

    if (sent_now == CONNECTION_WAS_CLOSED) {
      currConnState = ConnectionState::CONNECTION_CLOSED;
      return false;
    }
    if (sent_now == SEND_ERROR) {
      currConnState = ConnectionState::CONNECTION_FAILURE;
      return false;
    }
    sent += sent_now;
  }

  return true;
}
