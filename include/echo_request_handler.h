#ifndef GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_

#include <string>

#include <boost/beast/http.hpp>

#include "request_handler.h"

namespace http = boost::beast::http;
using http::string_body;

class EchoRequestHandler : public RequestHandler {
 public:
  EchoRequestHandler(std::string handled_directory_name);
  Status ParseRequest(const http::request<string_body>& req, http::response<string_body>& res) override;
};

#endif  // GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_
