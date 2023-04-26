#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "session.h"

using boost::asio::ip::tcp;

class SessionTest : public testing::Test {
 protected:
  SessionTest() : io_service_(), session_(io_service_), socket_(io_service_) {
  }

  virtual ~SessionTest() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  boost::asio::io_service io_service_;
  session session_;
  tcp::socket socket_;
};

TEST_F(SessionTest, TestHandleReadIncompleteRequest) {
  // Incomplete request (ends with \n)
  std::string incomplete_request = "GET /index.html HTTP/1.1\n";
  std::strcpy(session_.getData(), incomplete_request.c_str());
  boost::system::error_code ec;
  size_t bytes_transferred = incomplete_request.size();
  session_.handle_read(ec, bytes_transferred);

  std::string expected_message = "Request not valid, please enter a compete HTML request!\r\n";
  std::string actual_message(session_.getData(), expected_message.size());
  EXPECT_EQ(expected_message, actual_message);
}

TEST_F(SessionTest, TestHandleReadValidRequest) {
  // Valid request (ends with \r\n\r\n)
  std::string valid_request = "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:66.0) Gecko/20100101 Firefox/66.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\n\r\n";
  std::strcpy(session_.getData(), valid_request.c_str());
  boost::system::error_code ec;
  size_t bytes_transferred = valid_request.size();
  session_.handle_read(ec, bytes_transferred);

  std::string expected_response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\n" + valid_request;
  std::string actual_response(session_.getData(), expected_response.size());
  EXPECT_EQ(expected_response, actual_response);
}
/*
TEST_F(SessionTest, TestHandleWrite) {
  // Test handle_write() method
  std::string message = "Request not valid, please enter a compete HTML request!\r\n";
  std::strcpy(session_.getData(), message.c_str());
  boost::system::error_code ec;
  session_.handle_write(ec);

  // Verify that the socket is ready to read
  EXPECT_TRUE(socket_.is_open());
  EXPECT_FALSE(socket_.async_read_some(boost::asio::buffer(session_.getData(), 1024),
      std::bind(&session::handle_read, &session_,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred)));
}

TEST_F(SessionTest, TestStart) {
  // Test start() method
  boost::system::error_code ec;
  session_.start();
  // Verify that the socket is ready to read
  EXPECT_TRUE(socket_.is_open());
  EXPECT_FALSE(socket_.async_read_some(boost::asio::buffer(session_.data_, session_.max_length),
      boost::bind(&session::handle_read, &session
*/