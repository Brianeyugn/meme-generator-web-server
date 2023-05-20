#ifndef GOOFYGOOGLERSSERVER_SESSION_H_
#define GOOFYGOOGLERSSERVER_SESSION_H_

#include <string>

#include <boost/asio.hpp>

#include "config_parser.h"
#include "echo_request_handler.h"
#include "request_handler.h"
#include "static_request_handler.h"
#include "api_request_handler.h"
#include "request_factory.h"

using boost::asio::ip::tcp;

enum HandlerType {
  kStatic = 0,
  kEcho,
  kNone,
  kApi,
};

struct ParsedConfig {
  HandlerType handler_type;
  std::string url_prefix;
  std::shared_ptr<NginxConfigStatement> statement;
};

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
  std::string HandleRequest(const std::string request_string, 
      std::map<std::string, RequestHandlerFactory*>& routes);
  std::vector<ParsedConfig*> ParseConfigFile();
  void CreateHandlers(std::vector<ParsedConfig*>& parsed_configs, std::map<std::string, RequestHandlerFactory*>& factories);

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
