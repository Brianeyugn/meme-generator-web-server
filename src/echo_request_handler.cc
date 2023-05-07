#include "echo_request_handler.h"

#include <string>

EchoRequestHandler::EchoRequestHandler(std::string request_string, std::string handled_directory_name)
	: RequestHandler(request_string, handled_directory_name) {}

// Parse the echo request and update the response string
void EchoRequestHandler::ParseRequest() {  // Overide parent ParseRequest();
	// Echo handler returns echo no matter what.
	std::string response_status_code = "200 OK";
	std::string response_content_type = "text/plain";
  std::string file_contents = this->request_string_;

  // Response headers and message body.
  std::string response;
  response.append("HTTP/1.1 " + response_status_code + "\r\n");
  response.append("Content-Type: " + response_content_type + "\r\n");
  response.append("\r\n");
  response.append(file_contents);

  // Update response string.
  response_string_ = response;
}
