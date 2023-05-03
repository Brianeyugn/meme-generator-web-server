#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "session.h"
#include <fstream>
#include <sstream>
#include <iostream>

using boost::asio::ip::tcp;

class SessionHandleReadTest : public testing::Test {
 protected:
  SessionHandleReadTest() : io_service_(), session_(io_service_), socket_(io_service_) {
  }

  virtual ~SessionHandleReadTest() {
  }

  virtual void SetUp() {
    // Setup handlers
    erh1 = new echo_request_handler("", "echo");
    string base_1 = "../static_files/static_base_directory_1";
    string base_2 = "../static_files/static_base_directory_2";
    srh1 = new static_request_handler("", "static1", base_1);
    srh2 = new static_request_handler("", "static2", base_2);

    // Feed handlers to handler vector.
    handlers.push_back(erh1);
    handlers.push_back(srh1);
    handlers.push_back(srh2);

    config_file_ = std::ofstream("config_test.txt");
    config_file_ << "static /static1 ../static_files/static_base_directory_1\n";
    config_file_ << "echo /echo1\n";
    config_file_ << "invalid /invalid\n";
    config_file_.close();
    empty_file_ = std::ofstream("empty_file.txt");
    empty_file_.close();
    malconfig_file_ = std::ofstream("malconfig_test.txt");
    malconfig_file_ << "static static2\n";
    malconfig_file_ << "echo\n";
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
  session session_;
  tcp::socket socket_;
  std::ofstream config_file_;
  std::ofstream empty_file_;
  std::ofstream malconfig_file_;

  // Handlers
  echo_request_handler* erh1;
  static_request_handler* srh1;
  static_request_handler* srh2;

  vector<request_handler*> handlers;
  vector<request_handler*> handler_2;
};

// Test for handle_request() of the case of a using a static handler
TEST_F(SessionHandleReadTest, SessionHandlesReadRequestStatic) {
  std::string request_string = "GET /static1/test.html HTTP/1.1";
  std::string actual_response = session_.handle_request(request_string, handlers);
  std::string expected_response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 141\r\n\r\n<!DOCTYPE html>\n<html>\n<body style=\"background-color:powderblue;\">\n\n<h1>This is a heading</h1>\n<p>This is a paragraph.</p>\n\n</body>\n</html>\n\n";
  EXPECT_EQ(expected_response, actual_response);
}

// Test for handle_request() of the case of a using a default handler
TEST_F(SessionHandleReadTest, SessionHandlesReadRequestNotFound) {
  std::string request_string = "GET /not_available/test.html HTTP/1.1";
  std::string actual_response = session_.handle_request(request_string, handlers);
  std::string expected_response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 69\r\n\r\n404 Not Found. Error. The requested URL was not found on this server.";
  EXPECT_EQ(expected_response, actual_response);
}

// Tests for parse_config_file()

TEST_F(SessionHandleReadTest, TestParseConfigFile) {
    session_.parse_config_file("config_test.txt", handler_2);

    EXPECT_EQ(handler_2.size(), 2);

    static_request_handler* srh = dynamic_cast<static_request_handler*>(handler_2[0]);
    EXPECT_NE(srh, nullptr);

    echo_request_handler* erh = dynamic_cast<echo_request_handler*>(handler_2[1]);
    EXPECT_NE(erh, nullptr);
}

TEST_F(SessionHandleReadTest, TestParseConfigFileEmptyFile) {
    for (int i = 0; i < handler_2.size(); i++) {
      delete handler_2[i];
    }
    session_.parse_config_file("empty_file.txt", handler_2);

    EXPECT_EQ(handler_2.size(), 0);
}

TEST_F(SessionHandleReadTest, TestParseConfigFileNonexistentFile) {
    session_.parse_config_file("nonexistent.txt", handler_2);

    EXPECT_EQ(handler_2.size(), 0);
}

TEST_F(SessionHandleReadTest, TestParseConfigFileMalformedLine) {
    for (int i = 0; i < handler_2.size(); i++) {
        delete handler_2[i];
      }
    session_.parse_config_file("malconfig_test.txt", handler_2);

    EXPECT_EQ(handler_2.size(), 2);
}