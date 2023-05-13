#include "request_handler.h"

#include <string>

#include <boost/beast/http.hpp>

#include "gtest/gtest.h"

namespace http = boost::beast::http;
using http::string_body;

// Test static function GetNextToken for retrieval of consequtivly "/" delimited characters
TEST(RequestHandlerTest, GetNextTokenInStringPath) {
  std::string path_string = "this/is/a/fake/path";
  std::string delimiter = "/";

  std::string token_1 = RequestHandler::GetNextToken(path_string, delimiter);
  path_string.erase(0, token_1.size() + 1);
  std::string token_2 = RequestHandler::GetNextToken(path_string, delimiter);
  path_string.erase(0, token_2.size() + 1);
  std::string token_3 = RequestHandler::GetNextToken(path_string, delimiter);
  path_string.erase(0, token_3.size() + 1);
  std::string token_4 = RequestHandler::GetNextToken(path_string, delimiter);

  EXPECT_EQ(token_1, "this");
  EXPECT_EQ(token_2, "is");
  EXPECT_EQ(token_3, "a");
  EXPECT_EQ(token_4, "fake");
}

// Test static function GetRequestURL
TEST(RequestHandlerTest, GetRequestURLFromRequstLine) {
  std::string http_request_line = "GET /static1/test.html HTTP/1.1";
  std::string request_url = RequestHandler::GetRequestURL(http_request_line);
  EXPECT_EQ(request_url, "/static1/test.html");
}

// Test static function ContainsSubstring() when string exists.
TEST(RequestHandlerTest, ContainsSubstringFindsStringWhenExists) {
  std::string request_string = "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:90.0) Gecko/20100101 Firefox/90.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\nReferer: https://www.example.com/";
  std::string target_substr = "Connection: keep-alive";
  bool substr_exists = RequestHandler::ContainsSubstring(request_string, target_substr);
  EXPECT_EQ(substr_exists, true);
}

// Test static function ContainsSubstring() when substring does not exist.
TEST(RequestHandlerTest, ContainsSubstringNotFindsStringWhenNotExists) {
  std::string request_string = "This is just a random string";
  std::string target_substr = "Connection: keep-alive";
  bool substr_exists = RequestHandler::ContainsSubstring(request_string, target_substr);
  EXPECT_EQ(substr_exists, false);
}

// Test static function ContainsSubstring() when substring does not exist-- empty string edgecase.
TEST(RequestHandlerTest, ContainsSubstringNotFindsStringWhenEmptyString) {
  std::string request_string = "";
  std::string target_substr = "Connection: keep-alive";
  bool substr_exists = RequestHandler::ContainsSubstring(request_string, target_substr);
  EXPECT_EQ(substr_exists, false);
}

// Test static function ResponseToString()
TEST(RequestHandlerTest, ResponseToStringValidConversion) {
  //Set up boost string.
  http::response<string_body> boost_response;
  boost_response.version(11);
  boost_response.result(http::status::ok);
  boost_response.set(http::field::content_type, "text/plain");
  boost_response.body() = "Hello, world!";

  // Convert boost string to string
  std::string response_string = RequestHandler::ResponseToString(boost_response);

  // Compare actual to expected string values.
  std::string expected_response_string = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, world!";
  EXPECT_EQ(response_string, expected_response_string);
}