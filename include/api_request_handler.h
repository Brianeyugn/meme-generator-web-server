#ifndef GOOFYGOOGLERSSERVER_API_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_API_REQUEST_HANDLER_H_

#include <string>

#include <boost/beast/http.hpp>

#include "request_handler.h"

namespace http = boost::beast::http;
using http::string_body;

class ApiRequestHandler : public RequestHandler {
 public:
  ApiRequestHandler(std::string handled_directory_name, std::string base_directory_path);
  Status ParseRequest(const http::request<string_body>& req, http::response<string_body>& res) override;
 private:
  std::string base_directory_path_; // Relative path from Server program to base directory.
};

#endif // GOOFYGOOGLERSSERVER_API_REQUEST_HANDLER_H_
