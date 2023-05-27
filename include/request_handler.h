#ifndef GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_

#include <string>
#include <map>

#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>

namespace http = boost::beast::http;

// Abstract RequestHandler Class.
class RequestHandler {
 public:
  RequestHandler();
  virtual int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) = 0;
};

#endif  // GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_