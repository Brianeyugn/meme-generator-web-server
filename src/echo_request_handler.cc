#include "echo_request_handler.h"

#include <sstream>
#include <string>

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace http = boost::beast::http;
using http::string_body;

EchoRequestHandler::EchoRequestHandler(std::string handled_directory_name)
	: RequestHandler(handled_directory_name) {}

// Parse the echo request and update the response string
Status EchoRequestHandler::ParseRequest(const http::request<string_body>& req, http::response<string_body>& res) {  // Overide parent ParseRequest();
	// Echo handler returns echo no matter what.
  std::ostringstream oss;
  oss << req;
  std::string request_string = oss.str();

  // Response headers and message body.
  res.version(11); // HTTP/1.1
  res.result(http::status::ok); // 200 OK
  res.set(http::field::content_type, "text/plain");
  res.body() = request_string;

  // Error status return.
  Status return_status = Status(0, "Status Message: Success");
  return return_status;
}
