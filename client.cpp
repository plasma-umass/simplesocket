#include "simplesocket.h"
#include "clientsocket.h"

#include <pthread.h>
#include <sstream>
#include <iostream>

using namespace std;

int main (int argc, char * argv[])
{
  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " <port> <id>" << endl;
    exit (-1);
  }
  int port;
  int id;
  stringstream (argv[1]) >> port;
  stringstream (argv[2]) >> id;
  clientsocket * s = new clientsocket("127.0.0.1", port);
  s->connect();
  stringstream output;
  output << "CLIENT MESSAGE FROM " << id;
  cout << "client about to send: " << output.str().c_str() << endl;
  *s << output.str();
  string result;
  *s >> result;
  cout << "Received: {" << result << "}" << endl;

  s->close();
  return 0;
}
