#ifndef GOOFYGOOGLERSSERVER_REQUEST_FACTORY_H_
#define GOOFYGOOGLERSSERVER_REQUEST_FACTORY_H_

#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "api_request_handler.h"
#include "config_parser.h"
#include "echo_request_handler.h"
#include "error_request_handler.h"
#include "request_handler.h"
#include "static_request_handler.h"
#include "health_request_handler.h"

class RequestHandlerFactory {
 public:
  virtual RequestHandler* create(std::string location, NginxConfig* conf) = 0; 
};

class StaticRequestHandlerFactory : public RequestHandlerFactory {
 public:
  StaticRequestHandlerFactory();
  StaticRequestHandler* create(std::string location, NginxConfig* conf);
};

class EchoRequestHandlerFactory : public RequestHandlerFactory {
 public:
  EchoRequestHandlerFactory();
  EchoRequestHandler* create(std::string location, NginxConfig* conf);
};

class ErrorHandlerFactory : public RequestHandlerFactory {
 public:
  ErrorHandlerFactory();
  ErrorRequestHandler* create(std::string location, NginxConfig* conf);
};

class ApiRequestHandlerFactory : public RequestHandlerFactory {
 public:
  ApiRequestHandlerFactory();
  ApiRequestHandler* create(std::string location, NginxConfig* conf);
};

class HealthRequestHandlerFactory : public RequestHandlerFactory {
 public:
  HealthRequestHandlerFactory();
  HealthRequestHandler* create(std::string location, NginxConfig* conf);
};

#endif // GOOFYGOOGLERSSERVER_REQUEST_FACTORY_H_
