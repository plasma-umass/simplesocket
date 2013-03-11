// -*- C++ -*-

#ifndef HTTPMESSAGE_H
#define HTTPMESSAGE_H

#include "simplesocket.h"
#include <string>

using namespace std;

class HttpMessage
{
public:
  enum HttpMessageType
    {
      GET  = 0x0,
      POST = 0x1,
      INVALID = 0x2
    };
 
  HttpMessage (HttpMessageType in_type,
	       string in_resource,
	       string in_httpVersion)
  : MessageType (in_type),
    Resource (in_resource),
    HttpVersion (in_httpVersion)
  { }

  HttpMessage parseRequest (const string& in_message);
  void SendHttpMessage (simplesocket& s, char* in_data, size_t length);
  HttpMessage ReceiveHttpMessage (simplesocket& s);

  HttpMessageType MessageType;
  string Resource;
  string HttpVersion;
};

#endif
