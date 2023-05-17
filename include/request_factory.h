#ifndef GOOFYGOOGLERSSERVER_REQUEST_FACTORY_H_
#define GOOFYGOOGLERSSERVER_REQUEST_FACTORY_H_

#include <string>
#include <boost/asio.hpp>

#include "request_handler.h"
#include "static_request_handler.h"
#include "echo_request_handler.h"

class RequestHandlerFactory {
public:
  virtual RequestHandler* create(const std::string& location, const std::string& url);
   
};

class StaticRequestHandlerFactory : public RequestHandlerFactory {
public:
  StaticRequestHandlerFactory(std::string url) {this->url = url;}
  RequestHandler* create(const std::string& location, const std::string& url) override;
private:
  std::string url;
};

class EchoRequestHandlerFactory : public RequestHandlerFactory {
public:
  RequestHandler* create(const std::string& location, const std::string& url) override;
};

#endif // GOOFYGOOGLERSSERVER_REQUEST_FACTORY_H_