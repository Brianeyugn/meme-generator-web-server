#include "server.h"

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "config_parser.h"
#include "logging.h"
#include "session.h"

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_service& io_service, short port, 
               std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    handler_map_(handler_map) {
  Logger *log = Logger::GetLogger();
  log->LogInfo("Server :: Server: running on port: " + std::to_string(port));

  std::map<std::string, std::pair<std::string, NginxConfig*>>::iterator it;
  for (it = handler_map_.begin(); it != handler_map_.end(); it++) {
    std::string handler_type = it->second.first;

    routes_[it->first] = createHandlerFactory(handler_type);

    if (routes_[it->first] == nullptr) {
      log->LogWarn("Server :: Server: createHandlerFactory failed");
    } else {
      log->LogInfo("Server :: Server: createHandlerFactory success");
    }
  }
}

Server::~Server() {
  std::map<std::string, RequestHandlerFactory*>::iterator it;
  for (it = routes_.begin(); it != routes_.end(); it++) {
    if (it->second != nullptr) {
      delete it->second;
    }
  }
}

bool Server::StartAccept() {
  Session* new_session = new RealSession(io_service_, handler_map_, routes_);
  acceptor_.async_accept(new_session->Socket(),
    boost::bind(&Server::HandleAccept, this, new_session,
    boost::asio::placeholders::error));
  
  return new_session != nullptr;
}

// Start a new request Session
void Server::HandleAccept(Session* new_session,
  const boost::system::error_code& error) {
  if (!error) {
    new_session->Start();
  } else {
    delete new_session;
  }
  StartAccept();
}

RequestHandlerFactory* Server::createHandlerFactory(const std::string& name) {
  if (name == "EchoHandler") {
    return new EchoRequestHandlerFactory();
  }
  if (name == "StaticHandler") {
    return new StaticRequestHandlerFactory();
  }
  if (name == "ErrorHandler") {
    return new ErrorHandlerFactory();
  }
  if (name == "ApiHandler") {
    return new ApiRequestHandlerFactory();
  }
  if (name == "HealthHandler") {
    return new HealthRequestHandlerFactory();
  }
  if (name == "MemeHandler") {
    return new MemeRequestHandlerFactory();
  }
  return nullptr;
}
