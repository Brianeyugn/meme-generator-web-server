#ifndef GOOFYGOOGLERSSERVER_SESSION_H_
#define GOOFYGOOGLERSSERVER_SESSION_H_

#include <string>

#include <boost/asio.hpp>

#include "request_handler.h"
#include "echo_request_handler.h"
#include "static_request_handler.h"

using boost::asio::ip::tcp;
using namespace std;

class Session {
public:
  Session(boost::asio::io_service& io_service);
  tcp::socket& Socket();
  void Start();
  char* GetData();
  void HandleRead(const boost::system::error_code& error,
      size_t bytes_transferred);
  void HandleWrite(const boost::system::error_code& error);
  string HandleRequest(string request_string, 
      vector<RequestHandler*> handlers);
  void ParseConfigFile(const string& filename, 
      vector<RequestHandler*>& handlers);

private:
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  string read_string_buffer_;  // Accumulates all reads until a request is found.
};

#endif  // GOOFYGOOGLERSSERVER_
