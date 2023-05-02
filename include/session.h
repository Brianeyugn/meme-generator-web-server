#ifndef GOOFYGOOGLERSSERVER_SESSION_H_
#define GOOFYGOOGLERSSERVER_SESSION_H_

#include <string>
#include <boost/asio.hpp>

#include "request_handler.h"

using boost::asio::ip::tcp;
using namespace std;

class session {
public:
  session(boost::asio::io_service& io_service);
  tcp::socket& socket();
  void start();
  char* getData();
  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred);
  void handle_write(const boost::system::error_code& error);
  string handle_request(string request_string, vector<request_handler*> handlers);
  void parse_config_file(const string& filename, vector<request_handler*>& handlers);

private:
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  string read_string_buffer; // Accumulates all reads until a request is found.
};

#endif  // GOOFYGOOGLERSSERVER_