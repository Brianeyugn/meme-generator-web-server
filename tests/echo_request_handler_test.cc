#include "echo_request_handler.h"

#include <string>

#include "gtest/gtest.h"

// Test that echo_request_hander's ParseRequest formats the correct echo response.
TEST(EchoRequestHandlerTest, EchoParseRequstReturnsEcho) {
  EchoRequestHandler erh =  EchoRequestHandler("","echo");
  erh.SetRequestString("GET /echo/test.html HTTP/1.1");
  erh.ParseRequest();
  std::string response_string = erh.GetResponseString_();
  std::string expected_response_string = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGET /echo/test.html HTTP/1.1";
  EXPECT_EQ(response_string, expected_response_string);
}
