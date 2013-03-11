CXX = clang++
CXXFLAGS = -std=c++11 -g -I. # -Weverything

SRCS = server.cpp client.cpp 
LIBS = httpmessage.cpp simplesocket.cpp simplesocket.h serversocket.h clientsocket.h httpmessage.h

all: client server

client: client.cpp $(LIBS)
	$(CXX) $(CXXFLAGS) client.cpp simplesocket.cpp -o client

server: server.cpp $(LIBS)
	$(CXX) $(CXXFLAGS) server.cpp simplesocket.cpp -o server -lpthread

clean:
	rm -f server client

