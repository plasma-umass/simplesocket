// -*- C++ -*-

#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "simplesocket.h"

class clientsocket : public simplesocket {
public:
  clientsocket (const char * str,
		int port,
		int timeout = -1,
		bool debug = false)
    : simplesocket (str, port, timeout, debug)
  {
    initializeClient();
  }
   
private:
  // Clients can't serve or accept connections.
  void serve();
  simplesocket * accept();
};

#endif
