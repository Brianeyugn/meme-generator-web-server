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
#include "http_parser.h"

using boost::asio::ip::tcp;

class Session {
 public:
  Session(boost::asio::io_service& io_service, std::map<std::string, 
          std::pair<std::string, NginxConfig*>> handler_map,
          std::map<std::string, RequestHandlerFactory*> routes);

  virtual void Start() = 0;
  virtual tcp::socket& Socket() = 0;

 protected:
  virtual void HandleRead(const boost::system::error_code& error,
                          std::size_t bytes_transferred) = 0;

  virtual void HandleWrite(const boost::system::error_code& error) = 0;

  tcp::socket socket_;
  enum { max_length = 8192 };
  std::string client_ip_;
  char data_[max_length];

  std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map_;
  std::map<std::string, RequestHandlerFactory*> routes_;
  httpParser http_;
};

class RealSession : public Session {
 public:
  RealSession(boost::asio::io_service& io_service,
              std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map,
              std::map<std::string, RequestHandlerFactory*> routes);

  void Start();
  tcp::socket& Socket();

 private:
  void HandleRead(const boost::system::error_code& error, size_t bytes_transferred);

  void HandleWrite(const boost::system::error_code& error);

  std::string match(std::string request_uri);
};

#endif // GOOFYGOOGLERSSERVER_SESSION_H_
