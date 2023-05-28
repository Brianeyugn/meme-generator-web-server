#ifndef GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_

#include "request_handler.h"

#include <map>
#include <string>

#include "config_parser.h"

class StaticRequestHandler : public RequestHandler {
 public:
  StaticRequestHandler(const std::string& path, NginxConfig* config);
  int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res);
  
 private:
  std::string location_; // Config-specified location
  std::string root_; // Config-specified root
  bool bad_;
};

#endif // GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_
