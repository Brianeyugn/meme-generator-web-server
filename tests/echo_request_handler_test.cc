#include "echo_request_handler.h"

#include <string>

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;
using http::string_body;

#include "gtest/gtest.h"

// Test that echo_request_hander's ParseRequest formats the correct echo response.
TEST(EchoRequestHandlerTest, EchoParseRequstReturnsEcho) {
  // Initialize.
  EchoRequestHandler erh =  EchoRequestHandler("echo");

  // Set up request.
  std::string request_string = "GET /echo/test.html HTTP/1.1\r\n";
  http::request<string_body> boost_request = RequestHandler::StringToRequest(request_string);

  // Parse to obtain response.
  http::response<string_body> boost_response;
  erh.ParseRequest(boost_request, boost_response);

  // Compare actual response to expected response.
  std::string response_string = RequestHandler::ResponseToString(boost_response);
  std::string expected_response_string = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGET /echo/test.html HTTP/1.1\r\n\r\n";
  EXPECT_EQ(response_string, expected_response_string);
}
