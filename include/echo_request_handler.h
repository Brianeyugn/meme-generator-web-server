#ifndef GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_

#include <boost/beast/http.hpp>
#include "request_handler.h"

class EchoRequestHandler : public RequestHandler {
 public:
  EchoRequestHandler();
  int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res);
};

#endif  // GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_
