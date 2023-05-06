#ifndef GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_

#include "request_handler.h"

class EchoRequestHandler : public RequestHandler {
 public:
  EchoRequestHandler(string request_string, string handled_directory_name);
  void ParseRequest() override;
 private:
};

#endif  // GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_
