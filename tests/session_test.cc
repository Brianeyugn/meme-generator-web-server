#include "session.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <boost/asio.hpp>
#include <gtest/gtest.h>

using boost::asio::ip::tcp;
class SessionHandleReadTestFixture : public testing::Test {
 protected:
  NginxConfig config;
  SessionHandleReadTestFixture() : io_service_(), session_(io_service_, config), socket_(io_service_) {}

  virtual ~SessionHandleReadTestFixture() {}

  virtual void SetUp() {
    // Setup handlers
    erh1 = new EchoRequestHandler("echo");
    std::string base_1 = "../static_files/static_base_directory_1";
    std::string base_2 = "../static_files/static_base_directory_2";
    srh1 = new StaticRequestHandler("static1", base_1);
    srh2 = new StaticRequestHandler("static2", base_2);

    // Feed handlers to handler vector.
    handlers.push_back(erh1);
    handlers.push_back(srh1);
    handlers.push_back(srh2);

    config_file_ = std::ofstream("config_test.txt");
    config_file_ << "/static1 StaticHandler ../static_files/static_base_directory_1\n";
    config_file_ << "/echo1 EchoHandler\n";
    config_file_ << "/invalid invalid\n";
    config_file_.close();
    empty_file_ = std::ofstream("empty_file.txt");
    empty_file_.close();
    malconfig_file_ = std::ofstream("malconfig_test.txt");
    malconfig_file_ << "/static2 StaticHandler\n";
    malconfig_file_ << "EchoHandler\n";
    malconfig_file_.close();
  }

  virtual void TearDown() {
    // Clean up memory.
    for (int i = 0; i < handlers.size(); i++) {
      delete handlers[i];
    }
    for (int i = 0; i < handler_2.size(); i++) {
      delete handler_2[i];
    }

    std::remove("config_test.txt");
    std::remove("empty_file.txt");
    std::remove("malconfig_test.txt");
  }

  boost::asio::io_service io_service_;
  Session session_;
  tcp::socket socket_;
  std::ofstream config_file_;
  std::ofstream empty_file_;
  std::ofstream malconfig_file_;

  // Handlers
  EchoRequestHandler* erh1;
  StaticRequestHandler* srh1;
  StaticRequestHandler* srh2;

  std::vector<RequestHandler*> handlers;
  std::vector<RequestHandler*> handler_2;
};

/*
// Test for HandleRequest() of the case of a using a static handler
TEST_F(SessionHandleReadTestFixture, SessionHandlesReadRequestStatic) {
  std::string request_string = "GET /static1/test.html HTTP/1.1";
  std::string actual_response = session_.HandleRequest(request_string, handlers);
  std::string expected_response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 141\r\nConnection: keep-alive\r\n\r\n<!DOCTYPE html>\n<html>\n<body style=\"background-color:powderblue;\">\n\n<h1>This is a heading</h1>\n<p>This is a paragraph.</p>\n\n</body>\n</html>\n\n";
  EXPECT_EQ(expected_response, actual_response);
}

// Test for HandleRequest() of the case of a using a default handler
TEST_F(SessionHandleReadTestFixture, SessionHandlesReadRequestNotFound) {
  std::string request_string = "GET /not_available/test.html HTTP/1.1";
  std::string actual_response = session_.HandleRequest(request_string, handlers);
  std::string expected_response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 69\r\nConnection: keep-alive\r\n\r\n404 Not Found. Error. The requested URL was not found on this Server.";
  EXPECT_EQ(expected_response, actual_response);
}
*/

// Tests for ParseConfigFile()

TEST_F(SessionHandleReadTestFixture, TestParseConfigFile) {
    session_.ParseConfigFile("config_test.txt", handler_2);

    EXPECT_EQ(handler_2.size(), 2);

    StaticRequestHandler* srh = dynamic_cast<StaticRequestHandler*>(handler_2[0]);
    EXPECT_NE(srh, nullptr);

    EchoRequestHandler* erh = dynamic_cast<EchoRequestHandler*>(handler_2[1]);
    EXPECT_NE(erh, nullptr);
}

TEST_F(SessionHandleReadTestFixture, TestParseConfigFileEmptyFile) {
    for (int i = 0; i < handler_2.size(); i++) {
      delete handler_2[i];
    }
    session_.ParseConfigFile("empty_file.txt", handler_2);

    EXPECT_EQ(handler_2.size(), 0);
}

TEST_F(SessionHandleReadTestFixture, TestParseConfigFileNonexistentFile) {
    session_.ParseConfigFile("nonexistent.txt", handler_2);

    EXPECT_EQ(handler_2.size(), 0);
}

TEST_F(SessionHandleReadTestFixture, TestParseConfigFileMalformedLine) {
    for (int i = 0; i < handler_2.size(); i++) {
        delete handler_2[i];
      }
    session_.ParseConfigFile("malconfig_test.txt", handler_2);

    EXPECT_EQ(handler_2.size(), 1);
}