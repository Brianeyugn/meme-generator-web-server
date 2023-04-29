#include "gtest/gtest.h"
#include "static_request_handler.h"

// Test that get_filename() returns correct filename given short url.
TEST(StaticRequestHandlerTest, GetFilenameReturnFilenameForShortURL) {
  string url = "/test.html";
  string filename = static_request_handler::get_filename(url);
  EXPECT_EQ(filename, "test.html");
}

// Test that get_filename() returns correct filename given long url.
TEST(StaticRequestHandlerTest, GetFilenameReturnFilenameForLongURL) {
  string url = "/echo/test.html";
  string filename = static_request_handler::get_filename(url);
  EXPECT_EQ(filename, "test.html");
}

// Test that all content types of get_content_type() returns correct extension.
TEST(StaticRequestHandlerTest, GetContentTypeReturnsAllCorrectContentForEachExtension) {
  EXPECT_EQ(static_request_handler::get_content_type(".html"), "text/html");
  EXPECT_EQ(static_request_handler::get_content_type(".htm"), "text/html");
  EXPECT_EQ(static_request_handler::get_content_type(".txt"), "text/plain");
  EXPECT_EQ(static_request_handler::get_content_type(".jpg"), "image/jpeg");
  EXPECT_EQ(static_request_handler::get_content_type(".jpeg"), "image/jpeg");
  EXPECT_EQ(static_request_handler::get_content_type(".png"), "image/png");
  EXPECT_EQ(static_request_handler::get_content_type(".bin"), "application/octet-stream");
  EXPECT_EQ(static_request_handler::get_content_type(""), "application/octet-stream");
  EXPECT_EQ(static_request_handler::get_content_type(".zip"), "application/zip");
}

// Test of static_request_handler handling of a file that will not be found.
TEST(StaticRequestHandlerTest, StaticRequestHandlerReturnFileNotFoundWhenNotFound) {
  static_request_handler srh = static_request_handler("","static1","");
  string request_string = "GET /static1/filenotavailable.txt HTTP/1.1";
  srh.set_request_string_(request_string);
  srh.parse_request();
  string response_string = srh.get_response_string_();
  EXPECT_EQ(response_string, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 69\r\n\r\n404 Not Found. Error. The requested URL was not found on this server.");
}

// Test of static_request_handler handling of a file that will be found.
TEST(StaticRequestHandlerTest, StaticRequestHandlerReturnFileFoundWhenFound) {
  string base_1 = "../static_files/static_base_directory_1";
  static_request_handler srh = static_request_handler("","static1",base_1);
  string request_string = "GET /static1/smol.txt HTTP/1.1";
  srh.set_request_string_(request_string);
  srh.parse_request();
  string response_string = srh.get_response_string_();
  EXPECT_NE(response_string, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 69\r\n\r\n404 Not Found. Error. The requested URL was not found on this server.");
}