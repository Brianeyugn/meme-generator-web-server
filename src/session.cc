#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <iostream> // For Debugging
#include <cstring> // For strcpy
#include <sstream> // For iostringstream
#include "session.h"

using boost::asio::ip::tcp;

session::session(boost::asio::io_service& io_service)
  : socket_(io_service) {
}

tcp::socket& session::socket() {
  return socket_;
}

char* session::getData() {
  return data_;
}

void session::start() {
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred) {
  if (!error) {
    bool is_complete_request = false; // Is the HTML request complete?
    std::string data_string(data_,bytes_transferred);
    if (bytes_transferred >= 2) { // data_ read is 2 or more characters long
      if (data_string.substr(bytes_transferred - 2, 2) == "\n\n" ) {
        is_complete_request = true;
      }
      else if (bytes_transferred >= 4) { // data_ read is 4 or more characters long
        if (data_string.substr(bytes_transferred - 4, 4) == "\r\n\r\n") {
          is_complete_request = true;
        }
      }
    }

    if (is_complete_request == false) {
      std::string message = "Request not valid, please enter a compete HTML request!\r\n";
      int message_size = message.size();
      std::strcpy(data_, message.c_str());
      
      boost::asio::async_write(socket_,
        boost::asio::buffer(data_, message_size),
        boost::bind(&session::handle_write, this,
          boost::asio::placeholders::error));
    }
    else
    {

    std::string request = std::string(data_,bytes_transferred);
    std::istringstream iss(request);
    std::string method, request_uri, http_version;
    iss >> method >> request_uri >> http_version;
    
    std::string response = http_version + " 200 OK\r\nContent-Type: text/plain\r\n\n" + request;
    int response_size = response.size();
    std::strcpy(data_, response.c_str());

    boost::asio::async_write(socket_,
      boost::asio::buffer(data_, response_size),
      boost::bind(&session::handle_write, this,
        boost::asio::placeholders::error));
    }
  } else {
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error) {
  if (!error) {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  } else {
    delete this;
  }
}