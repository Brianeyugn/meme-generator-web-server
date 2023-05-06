#include "request_handler.h"

#include <iostream>

using namespace std;

RequestHandler::RequestHandler(string request_string, string handled_directory_name) {
  this->request_string_ = request_string;
  this->handled_directory_name_ = handled_directory_name;
}

// Given str and delimiter following directly after the str.
// Return Token.
string RequestHandler::GetNextToken(string str, string delimiter) {
  int token_length = str.find(delimiter, 0);
  string token = str.substr(0, token_length);
  return token;
}

// Extract the URL from the request
string RequestHandler::GetRequestURL(string request_string) {
  // Parse Request-Line.
  string method = GetNextToken(request_string, " ");
  request_string.erase(0, method.size() + 1);
  string request_url = GetNextToken(request_string, " ");
  return(request_url);
}

// Check whether reqeust_string_'s request_url request_directory_name matches with
// the serving handler's handled_directory_name;
bool RequestHandler::IsMatchingHandler()
{
  string request_url = GetRequestURL(this->request_string_);
  request_url.erase(0, 1); // strip root /.
  string request_directory_name = GetNextToken(request_url, "/");
  if (request_directory_name == this->handled_directory_name_) {
    return true;
  } else {
    return false;
  }
}

// Parse the request
void RequestHandler::ParseRequest() {
  // Return 404 Not Found since this is a simple base clase RequestHandler
  string response_status_code = "404 Not Found";
  string response_content_type = "text/plain";
  string file_contents = "404 Not Found. Error. The requested URL was not found on this Server.";
  string response_content_length = to_string(file_contents.size());

  // Response headers and message body.
  string response;
  response.append("HTTP/1.1 " + response_status_code + "\r\n");
  response.append("Content-Type: " + response_content_type + "\r\n");
  response.append("Content-Length: " + response_content_length + "\r\n");
  response.append("\r\n");
  response.append(file_contents);

  // Update response string.
  this->response_string_ = response;
}

// Must ParseRequest() first, then GetResponseString_().
string RequestHandler::GetResponseString_() {
  return this->response_string_;
}

string RequestHandler::GetRequestString_() {
  return this->request_string_;
}

void RequestHandler::SetRequestString(string request_string) {
  this->request_string_ = request_string;
}

void RequestHandler::SetResponseString(string response_string) {
  this->response_string_ = response_string;
}
