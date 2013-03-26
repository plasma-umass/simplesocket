// -*- C++ -*-

/**
 * @class  simplesocket
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 * @brief  A convenient wrapper for UNIX sockets.
 *
 **/

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
      _seenEof (false),
      _port (port),
      _hostname (str),
      _name (_hostname + string(":") + itos(_port)),
      _timeout (timeout),
      _debug (debug),
      _socketfd (-1)
  {}

  virtual ~simplesocket () {
    close();
  }

  /// @brief Write any value out to the socket.
  template <class TYPE>
  simplesocket& operator<<(TYPE t) {
    stringstream output;
    output << t;
    auto buf_str = output.str();
    auto buf = buf_str.c_str();
    write (buf, strlen(buf));
    return *this;
  }

  /// @brief Read in a string from the socket.
  simplesocket& operator>>(string& t) {
    char buf[MAXBUF+1];
    memset (buf, 0, MAXBUF+1);
    read (buf, MAXBUF);
    t = buf;
    return *this;
  }

  /// @brief True iff this socket is currently open.
  bool isOpen() const {
    return (_socketfd != -1);
  }

  /// @brief True iff this socket has read an EOF.
  bool seenEof() const {
    // seenEof => ! isOpen().
    assert (!_seenEof || !isOpen());
    return _seenEof;
  }

  /// @brief Write len bytes from a buffer into the socket.
  int write (const char * buf, size_t len) {
    return sendNBytes ((unsigned char*) buf, len, true);
  }

  /// @brief Read at most len bytes into the buffer from the socket.
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

  /// @brief The socket's hostname.
  const char * hostname() const {
    return _hostname.c_str();
  }

  /// @brief The socket port.
  int port() const {
    return _port;
  }

  /// @return true iff the name successfully resolved to an IP address.
  bool resolved (void) const {
    return _nameResolved;
  }

  /// @brief Close down the connection.
  void close();

  /// @brief Receive up to (or exactly) a number of bytes of data.
  ssize_t recvNBytes (void* data, int size, bool lessOk=false);

  /// @brief Send up to (or exactly) a number of bytes of data.
  ssize_t sendNBytes (unsigned char* data, int size, bool lessOk=false);

  /// @brief Establish a connection to a remote server.
  bool connect();

  /// @brief Act as a server and await connections.
  void serve();

  /// @brief Accept a connection.
  simplesocket * accept();

  void initializeClient() {
    // Prevent SIGPIPE signals from aborting the program.
    signal (SIGPIPE, SIG_IGN);
    resolve (NULL);
  }

private:

  void resolve (struct addrinfo * hint);
  int descriptorReady (int fd, int timeout);

  struct addrinfo * 	_addresses;
  int 			_socketfd;
  bool 			_nameResolved;
  bool 			_seenEof;
  const int 		_port;
  const string 		_hostname;
  const string 		_name;
  ostringstream 	_str;
  int 			_timeout;
  bool 			_debug;

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
