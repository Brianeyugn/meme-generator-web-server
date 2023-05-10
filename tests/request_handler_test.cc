#include "request_handler.h"

#include <string>

#include "gtest/gtest.h"

// Test set and get of request_string_for correctness.
TEST(RequestHandlerTest, SetGetRequestString) {
  RequestHandler rh =  RequestHandler("","");
  rh.SetRequestString("this is my test string");
  EXPECT_EQ(rh.GetRequestString_(), "this is my test string");
}

// Test set and get of response_string_for correctness.
TEST(RequestHandlerTest, SetGetResponseString) {
  RequestHandler rh =  RequestHandler("","");
  rh.SetRequestString("this is my test string");
  EXPECT_EQ(rh.GetRequestString_(), "this is my test string");
}

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

class RequstHandlerFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    return;
  }
  RequestHandler rh = RequestHandler("","static1");
};

// Test for Non-matching request to handler
TEST_F(RequstHandlerFixture, NonmatchingRequestDoesNotMatch) {
  rh.SetRequestString("GET /random_dir_name/test.html HTTP/1.1");
  bool result = rh.IsMatchingHandler();
  EXPECT_FALSE(result);
}

// Test for Matching request to handler
TEST_F(RequstHandlerFixture, MatchingRequestDoesMatch) {
  rh.SetRequestString("GET /static1/test.html HTTP/1.1");
  bool result = rh.IsMatchingHandler();
  EXPECT_TRUE(result);
}

// Test for RequestHandler ParseRequest returning not found as intended by default.
TEST_F(RequstHandlerFixture, RequestHandlerParseRequestReturnNotFound) {
  rh.SetRequestString("GET /static1/test.html HTTP/1.1");
  rh.ParseRequest();
  std::string response_string = rh.GetResponseString_();
  EXPECT_EQ(response_string, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 69\r\nConnection: keep-alive\r\n\r\n404 Not Found. Error. The requested URL was not found on this Server.");
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
