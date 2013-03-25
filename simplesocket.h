// -*- C++ -*-

#ifndef SIMPLESOCKET_H
#define SIMPLESOCKET_H

#include <signal.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cassert>
#include <string>
#include <string.h>		// for memset()
#include <sstream>
#include <errno.h>
#include <iostream>
#include <errno.h>
#include <sys/select.h>

using namespace std;

class simplesocket {

public:

  enum { TIMEOUT_ERR = -2};

public:

  simplesocket (const char * str,
		int port,
		int timeout = -1,
		bool debug = false)
    : _nameResolved (false),
      _seenEof(false),
      _port (port),
      _hostname (str),
      _name (_hostname + string(":") + itos(_port)),
      _timeout (timeout),
      _debug(debug)
  {}

  virtual ~simplesocket () {
    close();
  }

  /// Write any value out to the socket.
  template <class TYPE>
  simplesocket& operator<<(TYPE t) {
    stringstream output;
    output << t;
    auto buf_str = output.str();
    auto buf = buf_str.c_str();
    write (buf, strlen(buf));
    return *this;
  }

  /// Read in a string from the socket.
  simplesocket& operator>>(string& t) {
    char buf[MAXBUF+1];
    memset (buf, 0, MAXBUF+1);
    read (buf, MAXBUF);
    t = buf;
    return *this;
  }

  bool seenEof() const {
    return _seenEof;
  }

  int write (const char * buf, size_t len) {
    return sendNBytes ((unsigned char*) buf, len, true);
  }

  int read (char * buf, int len) {
    return recvNBytes(buf, len, true);
  }

  void setTimeout (int timeout) {
    if (timeout > 0)
      _timeout = timeout;
  }

  /// @brief The fully-qualified name (hostname + port).
  const char * name() const {
    return _name.c_str();
  }

  const char * hostname() const {
    return _hostname.c_str();
  }

  int port() const {
    return _port;
  }

  bool resolved (void) const {
    return _nameResolved;
  }

  void close();
  ssize_t recvNBytes (void* data, int size, bool lessOk=false);
  ssize_t sendNBytes (unsigned char* data, int size, bool lessOk=false);
  bool connect();
  void serve();
  simplesocket * accept();

  void initializeClient() {
    // Prevent SIGPIPE signals from aborting the program.
    signal (SIGPIPE, SIG_IGN);
    resolve (NULL);
  }

private:

  void resolve (struct addrinfo * hint);
  int descriptorReady (int fd, int timeout);

  struct addrinfo * _addresses;
  int _socketfd;
  bool _nameResolved;
  bool _seenEof;
  int _port;
  const string _hostname;
  const string _name;
  ostringstream _str;
  int _timeout;
  bool _debug;
   
  string itos (int v) {
    stringstream str;
    str << v;
    return str.str();
  }

  enum { MAXBUF = 65536 }; // largest possible input.

};

#ifndef MSG_NOSIGNAL
/* Operating systems which have SO_NOSIGPIPE but not MSG_NOSIGNAL */
#if defined (__FreeBSD__) || defined (__OpenBSD__) || defined(__APPLE__)
#define MSG_NOSIGNAL SO_NOSIGPIPE
#else
#error Your OS doesnt support MSG_NOSIGNAL or SO_NOSIGPIPE
#endif
#endif
 
#endif
