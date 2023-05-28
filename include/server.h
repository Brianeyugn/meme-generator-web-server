#ifndef GOOFYGOOGLERSSERVER_SERVER_H_
#define GOOFYGOOGLERSSERVER_SERVER_H_

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "session.h"
#include <string>
#include <map>
#include "echo_request_handler.h"
#include "static_request_handler.h"
#include "error_request_handler.h"
#include "api_request_handler.h"
#include "config_parser.h"
#include "request_factory.h"

using boost::asio::ip::tcp;

class Server {
 public:
  Server(boost::asio::io_service& io_service, short port,
        std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map);
  ~Server();
  bool StartAccept();

 private:
  void HandleAccept(Session* new_session,
      const boost::system::error_code& error);
  RequestHandlerFactory* createHandlerFactory(const std::string& name);

  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;

  std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map_;
  std::map<std::string, RequestHandlerFactory*> routes_;
};

#endif // GOOFYGOOGLERSSERVER_SERVER_H_
