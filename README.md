This directory contains wrapper classes meant to make life easier when
writing C++ code using sockets.

Basically, to put up a server, use a *serversocket*; to write a
client, use a *clientsocket*.  Each provide methods that let you use
sockets like ordinary C++ streams, including the `>>` and `<<`
operators.

The included server and client examples show how to use the API.
