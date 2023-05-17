#include "request_factory.h"

#include "echo_request_handler.h"
#include "request_handler.h"
#include "static_request_handler.h"

RequestHandler* RequestHandlerFactory::create(const std::string& location, const std::string& url) {
  return nullptr;
}

RequestHandler* StaticRequestHandlerFactory::create(const std::string& location, const std::string& url) {
  // Create and return a StaticRequestHandler instance
  // Pass necessary arguments to constructor
  return new StaticRequestHandler(location, this->url);
}

RequestHandler* EchoRequestHandlerFactory::create(const std::string& location, const std::string& url) {
  // Create and return an EchoRequestHandler instance
  // Pass necessary arguments to constructor
  return new EchoRequestHandler(location);
}