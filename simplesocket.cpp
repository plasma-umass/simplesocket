#include "simplesocket.h"
#include <unistd.h>
#include <fcntl.h>

void simplesocket::resolve (struct addrinfo * hint) {
  int err;
  int on = 1;
  
  char portStr[255];
  snprintf (portStr, sizeof(portStr), "%d", _port);
  
  if ((err = getaddrinfo (hostname(), portStr, hint, &_addresses)) != 0) {
    if (_debug)
      cerr << "Error in name resolution for " << name()
	   << ", with error " << gai_strerror(err) << "\n";
    _nameResolved = false;
    return;
  }
  
  _nameResolved = true;
  _socketfd = socket (AF_INET, SOCK_STREAM, 0);
  if (_socketfd < 0) {
    if (_debug)
      cerr << "SOCKET CREATION FAILURE... aborting.\n";
    return;
  }
  
  setsockopt (_socketfd, 
	      SOL_SOCKET,
	      SO_REUSEADDR, 
	      (void *) &on,
	      sizeof (on));
}

void simplesocket::close(){
  if (_socketfd > 0) {
    ::close (_socketfd);
    _socketfd = -1;
  }
}

int simplesocket::descriptorReady (int fd, int timeout) {
  assert (timeout > 0);
  int ret = 0;
  fd_set rset;
  FD_ZERO(&rset);
  FD_SET(fd, &rset);
  struct timeval tv;
  memset(&tv, 0, sizeof(struct timeval));
  tv.tv_sec = timeout;
  tv.tv_usec = 0;
  do {
    ret = select(fd + 1, &rset, NULL, NULL, &tv);
  } while (ret < 0 && errno == EINTR);
  return ret;
}

ssize_t simplesocket::recvNBytes(void* data, int size, bool lessOk){
  if (size<=0) return 0;
  int ret, nRecv = 0;
  int toRecv = size;
  do{
    if (_timeout > 0){
      ret = descriptorReady (_socketfd, _timeout);
      if (ret < 0){
	assert(errno > 0 && errno!=EINTR);
	close();
	return -errno;
      } else if (ret == 0) return (ssize_t)TIMEOUT_ERR;
    }
    if (_debug) cerr << "Descriptor ready, reading .. " << endl;
    ret = recv(_socketfd, data, toRecv, 0);
    if (_debug) cerr << "Recv returned error of .. " << ret << endl;
    if (ret<0 && errno == EINTR) continue;
    else if (ret<0) {
      assert(errno > 0);
      close();
      return -errno;
    } else if (ret == 0)  {
      _seenEof = true; 
      close();
      /* Close your side, abort connection */
      break; /* Other side send a fin, tell caller */
      /* Break with whatever data we could receive */
    } else {
      data = (void*) ((unsigned long) data + ret);
      toRecv -= ret;
      nRecv += ret;
    }
  } while(toRecv > 0 && !lessOk);
  return nRecv;
}

ssize_t simplesocket::sendNBytes (unsigned char* data, int size, bool lessOk) {
  int ret, nSent = 0;
  int toSend = size;
  assert (toSend>0);
  do {
    ret = ::send (_socketfd, data, toSend, MSG_NOSIGNAL);
    if (_debug) cerr << "sent " << ret << " bytes of " << size << endl;
    if ((ret < 0) && (errno == EINTR))
      continue; 
    else if (ret < 0){
      perror ("Error ");
      assert (errno > 0);
      close();
      return -errno;
    } else if (ret == 0) {
      close();
      return -1; /* Send shouldn't return 0 */
    } else {
      data += ret;
      toSend -= ret;
      nSent += ret;
    }
  } while (toSend > 0 && !lessOk);
  return nSent;
}

bool simplesocket::connect (void) {
  long arg;
  fd_set myset; 
  struct timeval tv; 
  int valopt; 
  socklen_t lon; 
  int res;

  if (!resolved()) return false;

  // Stolen from the web -mdc.
  // Set non-blocking 
  if( (arg = fcntl(_socketfd, F_GETFL, NULL)) < 0) { 
    fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
    exit(0); 
  } 
  arg |= O_NONBLOCK; 
  if( fcntl(_socketfd, F_SETFL, arg) < 0) { 
    fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
    exit(0); 
  } 
  // Trying to connect with timeout 
  res = ::connect(_socketfd, _addresses->ai_addr, _addresses->ai_addrlen); 
  if (res < 0) { 
    if (errno == EINPROGRESS) { 
      //      fprintf(stderr, "EINPROGRESS in connect() - selecting\n"); 
      do { 
	tv.tv_sec = 5; 
	tv.tv_usec = 0; 
	FD_ZERO(&myset); 
	FD_SET(_socketfd, &myset); 
	res = select(_socketfd+1, NULL, &myset, NULL, &tv); 
	if (res < 0 && errno != EINTR) { 
	  //fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
	  res =-1;
	  return (res >= 0);
	} 
	else if (res > 0) { 
	  // Socket selected for write 
	  lon = sizeof(int); 
	  if (getsockopt(_socketfd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
	    fprintf(stderr, "Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
	    exit (0);
	  } 
	  // Check the value returned... 
	  if (valopt) { 
	    //	    fprintf(stderr, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt)); 
	    res = -1;
	    return (res >= 0);
	  } 
	  break; 
	} 
	else { 
	  //	  fprintf(stderr, "Timeout in select() - Cancelling!\n"); 
	  res = -1;
	  return (res >= 0);
	} 
      } while (true); 
    } 
    else { 
      //fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
      res = -1;
      return (res >= 0);
    } 
  } 
  // Set to blocking mode again... 
  if( (arg = fcntl(_socketfd, F_GETFL, NULL)) < 0) { 
    fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
    exit(0); 
  } 
  arg &= (~O_NONBLOCK); 
  if( fcntl(_socketfd, F_SETFL, arg) < 0) { 
    fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
    exit(0); 
  } 
  
  /*  if (res < 0) {
      if (_debug) cerr << "NO CONNECTION, errno = " << errno << endl;
      }*/
  return (res >= 0);
}

void simplesocket::serve (void) 
{
  struct addrinfo hint;
  memset ((void *) &hint, 0, sizeof(hint));
  hint.ai_flags = AI_PASSIVE;
  resolve (&hint);

  ((struct sockaddr_in*)(_addresses->ai_addr))->sin_addr.s_addr = htonl(INADDR_ANY);
  int r = bind (_socketfd,
		_addresses->ai_addr,
		_addresses->ai_addrlen);
  if (r < 0) {
    perror ("Bind failure.");
  }
  if (_debug) cerr << "waiting for a connection - " << _name << endl;
  r = listen (_socketfd, 255);
  if (r < 0) {
    if (_debug) cerr << "LISTEN FAILURE.\n";
  }
}

simplesocket * simplesocket::accept() {
  socklen_t len = sizeof(_addresses);
  int newfd = ::accept (_socketfd, _addresses->ai_addr, &len);
  if (newfd < 0) {
    if (_debug) cerr << "ACCEPT FAILURE.\n";
    return NULL;
  }
  simplesocket * newSock = new simplesocket (_hostname.c_str(), _port, -1, _debug);
  newSock->_socketfd = newfd;
  newSock->_nameResolved = true;
  return newSock;
}

