#include "request_handler.h"

#include <string>

RequestHandler::RequestHandler(std::string request_string, std::string handled_directory_name)
  : request_string_(request_string), handled_directory_name_(handled_directory_name) {}

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

// Check whether reqeust_string_'s request_url request_directory_name matches with
// the serving handler's handled_directory_name;
bool RequestHandler::IsMatchingHandler() {
  std::string request_url = GetRequestURL(this->request_string_);
  request_url.erase(0, 1); // strip root /.
  std::string request_directory_name = GetNextToken(request_url, "/");
  return request_directory_name == this->handled_directory_name_;
}

// Parse the request
void RequestHandler::ParseRequest() {
  // Return 404 Not Found since this is a simple base clase RequestHandler
  std::string response_status_code = "404 Not Found";
  std::string response_content_type = "text/plain";
  std::string file_contents = "404 Not Found. Error. The requested URL was not found on this Server.";
  std::string response_content_length = std::to_string(file_contents.size());

  // Response headers and message body.
  std::string response;
  response.append("HTTP/1.1 " + response_status_code + "\r\n");
  response.append("Content-Type: " + response_content_type + "\r\n");
  response.append("Content-Length: " + response_content_length + "\r\n");
  response.append("\r\n");
  response.append(file_contents);

  // Update response string.
  this->response_string_ = response;
}
