#ifndef GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_

#include "request_handler.h"

#include <string>

class EchoRequestHandler : public RequestHandler {
 public:
  EchoRequestHandler(std::string request_string, std::string handled_directory_name);
  void ParseRequest() override;
};

#endif  // GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_
