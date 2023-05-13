#ifndef GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_

#include <string>

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;
using http::string_body;

// Define struct to represent Status
struct Status {
  int code_; // 0 represents success.
  std::string message_;

  Status(int code, const std::string message) : code_(code), message_(message) {}
};


// Abstract RequestHandler Class.
class RequestHandler {
 public:
  RequestHandler(std::string handled_directory_name);

  virtual Status ParseRequest(const http::request<string_body>& req, http::response<string_body>& res) = 0;

  static std::string GetNextToken(std::string str, std::string delimiter);
  static std::string GetRequestURL(std::string request_string);
  static bool ContainsSubstring(const std::string&, const std::string& substr);
  static http::request<string_body> StringToRequest(std::string request_string);
  static std::string ResponseToString(http::response<string_body> boost_response);
 protected:
  std::string handled_directory_name_;
};

#endif  // GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_