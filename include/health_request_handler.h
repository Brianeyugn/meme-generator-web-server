#ifndef GOOFYGOOGLERSSERVER_HEALTH_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_HEALTH_REQUEST_HANDLER_H_

#include "request_handler.h"

#include <string>

#include "config_parser.h"

class HealthRequestHandler : public RequestHandler {
 public:
  HealthRequestHandler(const std::string& path, NginxConfig* config);
  int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res);
  
 private:
  std::string location_; // Config-specified location
};

#endif // GOOFYGOOGLERSSERVER_HEALTH_REQUEST_HANDLER_H_