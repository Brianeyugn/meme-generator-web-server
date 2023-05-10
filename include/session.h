#ifndef GOOFYGOOGLERSSERVER_SESSION_H_
#define GOOFYGOOGLERSSERVER_SESSION_H_

#include <string>

#include <boost/asio.hpp>

#include "config_parser.h"
#include "echo_request_handler.h"
#include "request_handler.h"
#include "static_request_handler.h"

using boost::asio::ip::tcp;

class Session {
 public:
  Session(boost::asio::io_service& io_service, const NginxConfig& config);

  void Start();
  char* GetData();
  tcp::socket& GetSocket();

  void HandleRead(const boost::system::error_code& error,
      std::size_t bytes_transferred);
  void HandleWrite(const boost::system::error_code& error);
  void HandleWriteShutdown(const boost::system::error_code& error);
  std::string HandleRequest(std::string request_string, 
      std::vector<RequestHandler*> handlers);
  void ParseConfigFile(std::vector<RequestHandler*>& handlers);

  // temporary overloading
  void ParseConfigFile(const std::string& filename, 
      std::vector<RequestHandler*>& handlers);

 private:
  enum { max_length = 1024 };
  char data_[max_length];
  std::string read_string_buffer_;  // Accumulates all reads until a request is found.
  tcp::socket socket_;
  NginxConfig config_;
};

inline char* Session::GetData() {
  return data_;
}

inline tcp::socket& Session::GetSocket() {
  return socket_;
}

#endif  // GOOFYGOOGLERSSERVER_
