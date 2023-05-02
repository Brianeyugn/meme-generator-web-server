#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "session.h"
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
  }

  virtual void TearDown() {
    // Clean up memory.
    for (int i = 0; i < handlers.size(); i++) {
      delete handlers[i];
    }
  }

  boost::asio::io_service io_service_;
  session session_;
  tcp::socket socket_;

  // Handlers
  echo_request_handler* erh1;
  static_request_handler* srh1;
  static_request_handler* srh2;

  vector<request_handler*> handlers;
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



