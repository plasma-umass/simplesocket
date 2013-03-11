// -*- C++ -*-

#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include "simplesocket.h"

class serversocket : public simplesocket {
public:
  serversocket (int port, int timeout = -1, bool debug = false)
    : simplesocket ("127.0.0.1", port, timeout, debug)
  {
    serve();
  }
   
private:

  // Servers don't act as clients.
  void initializeClient();

  // Servers can't make new outgoing connections.
  bool connect();
};

#endif
