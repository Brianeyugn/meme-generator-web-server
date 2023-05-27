#ifndef GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_

#include <string>
#include <map>
#include "config_parser.h"
#include "request_handler.h"


class StaticRequestHandler : public RequestHandler {
 public:
  StaticRequestHandler(const std::string& path, NginxConfig* config);
  int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res);
  
 private:
  std::string location; // Config-specified location
  std::string root; // Config-specified root
  bool bad;
};

#endif // GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_
