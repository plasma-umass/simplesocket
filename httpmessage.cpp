#include <iostream>
#include "httpmessage.h"

using namespace std;

HttpMessage HttpMessage::ReceiveHttpMessage (simplesocket& sock) 
{
  string message;
  string s;
  char temp;
  while (true){
    sock.recvNBytes(&temp, 1, false);
    s = temp;
    message += s;
    if (temp == 0xD)
      {
	sock.recvNBytes(&temp,1, false);
	s = temp;
	message += s;
	if (temp == 0xA)
	  break;
      }
  }

  HttpMessage ret = parseRequest (message);
  sock.recvNBytes(&temp, 1, false);
  s = temp;
  message += s;
  if (temp == 0xD) {
    sock.recvNBytes(&temp,1, false);
    s = temp;
    message += s;
    if (temp == 0xA)
      {
	return ret;
      }
  }
  while (true) {
    sock.recvNBytes(&temp, 1, false);
    s = temp;
    message += s;
    if (temp == 0xD) {
      sock.recvNBytes(&temp,1, false);
      s = temp;
      message += s;
      if (temp == 0xA){
	sock.recvNBytes(&temp,1, false);
	s = temp;
	message += s;
	if (temp == 0xD){
	  sock.recvNBytes(&temp,1, false);
	  s = temp;
	  message += s;
	  if (temp == 0xA)
	    {
	      if (_debug)
		cerr << "Full message: " << message << endl;
	      return ret;
	    }
	}
      }
    }
  }
}

void HttpMessage::SendHttpMessage (simplesocket& sock,
				   char * in_data,
				   size_t length)
{
  char CR =0xD;
  char LF =0xA;
  string ResponseHeader = "HTTP/1.1 200 OK";
  ResponseHeader.push_back(CR);
  ResponseHeader.push_back(LF);
  
  string ContentTypeHeader = "Content-Type: text/html";
  ContentTypeHeader.push_back(CR);
  ContentTypeHeader.push_back(LF);
  ContentTypeHeader.push_back(CR);
  ContentTypeHeader.push_back(LF);
  
  string resource(in_data, length);
  resource.push_back(CR);
  resource.push_back(LF);
  resource.push_back(CR);
  resource.push_back(LF);
  
  
  ResponseHeader += ContentTypeHeader;
  ResponseHeader += resource;
  
  //      unsigned char* ptr = (unsigned char*)ResponseHeader.c_str();
  sock.sendNBytes( (unsigned char*)ResponseHeader.c_str(), ResponseHeader.length());
}

HttpMessage HttpMessage::parseRequest (const string& in_message)
{
  string message = in_message;
  int position = message.find(" ");
  string type = message.substr(0, position);
  message = message.substr(position+1);
  
  //next space
  position = message.find(" ");
  string request = message.substr(0, position);
  message = message.substr(position+1);
  
  //next delimiter
  string endmarker;
  endmarker.push_back('\r');
  endmarker.push_back('\n');
  position = message.find(endmarker);
  string httpversion = message.substr(0, position);
  
  HttpMessage::HttpMessageType temphttptype;
  if (type != "GET")
    temphttptype = HttpMessage::INVALID;
  else
    temphttptype = HttpMessage::GET;
  
  HttpMessage ret (temphttptype, request, httpversion);
  return ret;
}
