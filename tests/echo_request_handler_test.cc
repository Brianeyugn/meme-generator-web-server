#include "gtest/gtest.h"
#include "echo_request_handler.h"

// Test that echo_request_hander's parse_request formats the correct echo response.
TEST(EchoRequestHandlerTest, EchoParseRequstReturnsEcho) {
  echo_request_handler erh =  echo_request_handler("","echo");
  erh.set_request_string_("GET /echo/test.html HTTP/1.1");
  erh.parse_request();
  string response_string = erh.get_response_string_();
  string expected_response_string = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGET /echo/test.html HTTP/1.1";
  EXPECT_EQ(response_string, expected_response_string);
}