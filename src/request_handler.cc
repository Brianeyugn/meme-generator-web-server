#include "request_handler.h"

#include <iostream>

using namespace std;

request_handler::request_handler(string request_string, string handled_directory_name) {
  this->request_string_ = request_string;
  this->handled_directory_name_ = handled_directory_name;
}

// Given str and delimiter following directly after the str.
// Return Token.
string request_handler::get_next_token(string str, string delimiter) {
  int token_length = str.find(delimiter, 0);
  string token = str.substr(0, token_length);
  return token;
}

string request_handler::get_request_url(string request_string) {
  // Parse Request-Line.
  string method = get_next_token(request_string, " ");
  request_string.erase(0, method.size() + 1);
  string request_url = get_next_token(request_string, " ");
  return(request_url);
}

// Check whether reqeust_string_'s request_url request_directory_name matches with
//  the serving handler's handled_directory_name;
bool request_handler::is_matching_handler()
{
  string request_url = get_request_url(this->request_string_);
  request_url.erase(0, 1); // strip root /.
  string request_directory_name = get_next_token(request_url, "/");
  if (request_directory_name == this->handled_directory_name_) {
    return true;
  } else {
    return false;
  }
}

void request_handler::parse_request() {
  // Return 404 Not Found since this is a simple base clase request_handler
  string response_status_code = "404 Not Found";
  string response_content_type = "text/plain";
  string file_contents = "404 Not Found. Error. The requested URL was not found on this server.";
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

string request_handler::get_response_string_() {
  return this->response_string_;
}

string request_handler::get_request_string_() {
  return this->request_string_;
}

void request_handler::set_request_string_(string request_string) {
  this->request_string_ = request_string;
}

void request_handler::set_response_string_(string response_string) {
  this->response_string_ = response_string;
}
