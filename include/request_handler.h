#ifndef GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_

#include <map>
#include <string>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#define HTTP_STATUS_OK      200
#define HTTP_STATUS_CREATED 201

#define HTTP_STATUS_BAD_REQUEST 400
#define HTTP_STATUS_NOT_FOUND   404

#define HTTP_STATUS_INTERNAL_SERVER_ERROR 500

namespace http = boost::beast::http;

// Abstract RequestHandler Class.
class RequestHandler {
 public:
  RequestHandler();
  virtual int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) = 0;

 protected:
  int handle_bad_request(http::response<http::string_body>& res);
  int handle_not_found(http::response<http::string_body>& res);
  int handle_internal_server_error(http::response<http::string_body>& res);
};

inline int RequestHandler::handle_bad_request(http::response<http::string_body>& res) {
  res.reason("Bad Request");
  res.result(HTTP_STATUS_BAD_REQUEST);
  return HTTP_STATUS_BAD_REQUEST;
}

inline int RequestHandler::handle_not_found(http::response<http::string_body>& res) {
  std::string error_msg = "404 Not Found\r\n";
  res.set(http::field::content_type, "text/plain");
  res.set(http::field::content_length, std::to_string(error_msg.size()));
  res.body() = error_msg;
  res.reason("Not Found");
  res.result(HTTP_STATUS_NOT_FOUND);
  return HTTP_STATUS_NOT_FOUND;
}

inline int RequestHandler::handle_internal_server_error(http::response<http::string_body>& res) {
  res.reason("Internal Server Error");
  res.result(HTTP_STATUS_INTERNAL_SERVER_ERROR);
  return HTTP_STATUS_INTERNAL_SERVER_ERROR;
}

#endif // GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_