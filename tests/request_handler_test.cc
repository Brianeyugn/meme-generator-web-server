#include "gtest/gtest.h"
#include "request_handler.h"

// Test set and get of request_string_for correctness.
TEST(RequestHandlerTest, SetGetRequestString) {
  request_handler rh =  request_handler("","");
  rh.set_request_string_("this is my test string");
  EXPECT_EQ(rh.get_request_string_(), "this is my test string");
}

// Test set and get of response_string_for correctness.
TEST(RequestHandlerTest, SetGetResponseString) {
  request_handler rh =  request_handler("","");
  rh.set_request_string_("this is my test string");
  EXPECT_EQ(rh.get_request_string_(), "this is my test string");
}

// Test static function get_next_token for retrieval of consequtivly "/" delimited characters
TEST(RequestHandlerTest, GetNextTokenInStringPath) {
  string path_string = "this/is/a/fake/path";
  string delimiter = "/";

  string token_1 = request_handler::get_next_token(path_string, delimiter);
  path_string.erase(0, token_1.size() + 1);
  string token_2 = request_handler::get_next_token(path_string, delimiter);
  path_string.erase(0, token_2.size() + 1);
  string token_3 = request_handler::get_next_token(path_string, delimiter);
  path_string.erase(0, token_3.size() + 1);
  string token_4 = request_handler::get_next_token(path_string, delimiter);

  EXPECT_EQ(token_1, "this");
  EXPECT_EQ(token_2, "is");
  EXPECT_EQ(token_3, "a");
  EXPECT_EQ(token_4, "fake");
}

// Test static function get_request_url
TEST(RequestHandlerTest, GetRequestURLFromRequstLine) {
  string http_request_line = "GET /static1/test.html HTTP/1.1";
  string request_url = request_handler::get_request_url(http_request_line);
  EXPECT_EQ(request_url, "/static1/test.html");
}

class RequstHandlerFixture : public ::testing::Test {
  protected:
    void SetUp() override {
      return;
    }
  request_handler rh = request_handler("","static1");
};

// Test for Non-matching request to handler
TEST_F(RequstHandlerFixture, NonmatchingRequestDoesNotMatch) {
  rh.set_request_string_("GET /random_dir_name/test.html HTTP/1.1");
  bool result = rh.is_matching_handler();
  EXPECT_FALSE(result);
}

// Test for Matching request to handler
TEST_F(RequstHandlerFixture, MatchingRequestDoesMatch) {
  rh.set_request_string_("GET /static1/test.html HTTP/1.1");
  bool result = rh.is_matching_handler();
  EXPECT_TRUE(result);
}

// Test for request_handler parse_request returning not found as intended by default.
TEST_F(RequstHandlerFixture, RequestHandlerParseRequestReturnNotFound) {
  rh.set_request_string_("GET /static1/test.html HTTP/1.1");
  rh.parse_request();
  string response_string = rh.get_response_string_();
  EXPECT_EQ(response_string, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 69\r\n\r\n404 Not Found. Error. The requested URL was not found on this server.");
}