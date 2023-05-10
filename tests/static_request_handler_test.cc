#include "static_request_handler.h"

#include "gtest/gtest.h"

/*
// Test that GetFilename() returns correct filename given short url.
TEST(StaticRequestHandlerTest, GetFilenameReturnFilenameForShortURL) {
  std::string url = "/test.html";
  std::string filename = StaticRequestHandler::GetFilename(url);
  EXPECT_EQ(filename, "test.html");
}

// Test that GetFilename() returns correct filename given long url.
TEST(StaticRequestHandlerTest, GetFilenameReturnFilenameForLongURL) {
  std::string url = "/echo/test.html";
  std::string filename = StaticRequestHandler::GetFilename(url);
  EXPECT_EQ(filename, "test.html");
}

// Test that all content types of GetContentType() returns correct extension.
TEST(StaticRequestHandlerTest, GetContentTypeReturnsAllCorrectContentForEachExtension) {
  EXPECT_EQ(StaticRequestHandler::GetContentType(".html"), "text/html");
  EXPECT_EQ(StaticRequestHandler::GetContentType(".htm"), "text/html");
  EXPECT_EQ(StaticRequestHandler::GetContentType(".txt"), "text/plain");
  EXPECT_EQ(StaticRequestHandler::GetContentType(".jpg"), "image/jpeg");
  EXPECT_EQ(StaticRequestHandler::GetContentType(".jpeg"), "image/jpeg");
  EXPECT_EQ(StaticRequestHandler::GetContentType(".png"), "image/png");
  EXPECT_EQ(StaticRequestHandler::GetContentType(".bin"), "application/octet-stream");
  EXPECT_EQ(StaticRequestHandler::GetContentType(""), "application/octet-stream");
  EXPECT_EQ(StaticRequestHandler::GetContentType(".zip"), "application/zip");
}
*/

// Test of StaticRequestHandler handling of a file that will not be found.
TEST(StaticRequestHandlerTest, StaticRequestHandlerReturnFileNotFoundWhenNotFound) {
  StaticRequestHandler srh = StaticRequestHandler("","static1","");
  std::string request_string = "GET /static1/filenotavailable.txt HTTP/1.1";
  srh.SetRequestString(request_string);
  srh.ParseRequest();
  std::string response_string = srh.GetResponseString_();
  EXPECT_EQ(response_string, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 69\r\nConnection: keep-alive\r\n\r\n404 Not Found. Error. The requested URL was not found on this Server.");
}

// Test of StaticRequestHandler handling of a file that will be found.
TEST(StaticRequestHandlerTest, StaticRequestHandlerReturnFileFoundWhenFound) {
  std::string base_1 = "../static_files/static_base_directory_1";
  StaticRequestHandler srh = StaticRequestHandler("","static1",base_1);
  std::string request_string = "GET /static1/smol.txt HTTP/1.1";
  srh.SetRequestString(request_string);
  srh.ParseRequest();
  std::string response_string = srh.GetResponseString_();
  EXPECT_EQ(response_string, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 5\r\nConnection: keep-alive\r\n\r\nhello");
}
