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
  // Initialize.
  StaticRequestHandler srh = StaticRequestHandler("static1","");

  // Set up request.
  std::string request_string = "GET /static1/filenotavailable.txt HTTP/1.1";
  http::request<string_body> boost_request = RequestHandler::StringToRequest(request_string);

  // Parse to obtain response.
  http::response<string_body> boost_response;
  srh.ParseRequest(boost_request, boost_response);

  // Compare actual response to expected response.
  std::string response_string = RequestHandler::ResponseToString(boost_response);
  std::string expected_response_string = "HTTP/1.1 404 Not Found\r\nConnection: keep-alive\r\nContent-Type: text/plain\r\nContent-Length: 69\r\n\r\n404 Not Found. Error. The requested URL was not found on this server.";
  EXPECT_EQ(response_string, expected_response_string);
}

// Test of StaticRequestHandler handling of a file that will be found.
TEST(StaticRequestHandlerTest, StaticRequestHandlerReturnFileFoundWhenFound) {
  // Initialize.
  std::string base_1 = "../static_files/static_base_directory_1";
  StaticRequestHandler srh = StaticRequestHandler("static1",base_1);

  // Set up request.
  std::string request_string = "GET /static1/smol.txt HTTP/1.1\r\n";
  http::request<string_body> boost_request = RequestHandler::StringToRequest(request_string);

  // Parse to obtain response.
  http::response<string_body> boost_response;
  srh.ParseRequest(boost_request, boost_response);

  // Compare actual response to expected response.
  std::string response_string = RequestHandler::ResponseToString(boost_response);
  std::string expected_response_string = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nhello";
  EXPECT_EQ(response_string, expected_response_string);
}
