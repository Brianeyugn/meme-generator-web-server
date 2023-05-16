#include "request_handler.h"

#include <sstream>
#include <string>

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;
using http::string_body;

RequestHandler::RequestHandler(std::string handled_directory_name)
  : handled_directory_name_(handled_directory_name) {}

// Given str and delimiter following directly after the str.
// Return Token.
std::string RequestHandler::GetNextToken(std::string str, std::string delimiter) {
  int token_length = str.find(delimiter, 0);
  std::string token = str.substr(0, token_length);
  return token;
}

// Extract the URL from the request
std::string RequestHandler::GetRequestURL(std::string request_string) {
  // Parse Request-Line.
  std::string method = GetNextToken(request_string, " ");
  request_string.erase(0, method.size() + 1);
  std::string request_url = GetNextToken(request_string, " ");
  return request_url;
}

// Given str and substr to be found.
// Returns true if substr is found in str, returns false if not found.
bool RequestHandler::ContainsSubstring(const std::string& str, const std::string& substr) {
  return (str.find(substr) != std::string::npos);
}

// Given a std string.
// Returns a http boost request from std string data.
http::request<http::string_body> RequestHandler::StringToRequest(std::string request_string)
{
  boost::beast::error_code error;
  http::request_parser<http::string_body> parser;

  // Parse headers.
  boost::asio::const_buffer buffer = boost::asio::buffer(request_string);
  std::size_t bytes_parsed = parser.put(buffer, error);

  // Parse body.
  buffer += bytes_parsed;
  bytes_parsed = parser.put(buffer, error);
  parser.put_eof(error);

  return parser.release();
}

// Given a http boost response.
// Returns a std string based on te http boost response data.
std::string RequestHandler::ResponseToString(http::response<string_body> boost_response) {
  std::ostringstream oss;
  oss << boost_response;
  std::string response_string = oss.str();
  return response_string;
}

// Parse the request
Status RequestHandler::ParseRequest(const http::request<string_body>& req, http::response<string_body>& res) {
  // Convert request to string format.
  std::ostringstream oss;
  oss << req;
  std::string request_string = oss.str();

  // Response Components.
  std::string response_content_type;
  std::string file_contents;

  res.result(boost::beast::http::status::not_found); // 404 Not Found.
  response_content_type = "text/plain";
  file_contents = "404 Not Found. Error. The requested URL was not found on this server.";

  // Check if connection keep alive demanded
  bool keep_alive_request_exists = req.keep_alive();
  if (keep_alive_request_exists == true) {
    res.set(http::field::connection, "keep-alive");
  } else {
    res.set(http::field::connection, "close");
  }

  // Response headers and message body.
  res.set(http::field::content_type, response_content_type);
  res.body() = file_contents;
  res.prepare_payload(); // Adjusts Content-Length and Transfer-Encoding field values based on body properties.

  // Error Status return.
  Status return_status = Status(0, "Status Message: Success");
  return return_status;
}