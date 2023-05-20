#include "request_factory.h"

#include "request_handler.h"
#include "echo_request_handler.h"
#include "static_request_handler.h"
#include "api_request_handler.h"

RequestHandler* RequestHandlerFactory::create(const std::string& location, const std::string& url) {
  return nullptr;
}

StaticRequestHandler* StaticRequestHandlerFactory::create(const std::string& location, const std::string& url) {
  // Create and return a StaticRequestHandler instance
  // Pass necessary arguments to constructor
  return new StaticRequestHandler(location, url);
}

EchoRequestHandler* EchoRequestHandlerFactory::create(const std::string& location, const std::string& url) {
  // Create and return an EchoRequestHandler instance
  // Pass necessary arguments to constructor
  return new EchoRequestHandler(location);
}

ApiRequestHandler* ApiRequestHandlerFactory::create(const std::string& location, const std::string& url) {
  // Create and return an ApiRequestHandler instance
  // Pass necessary arguments to constructor
  return new ApiRequestHandler(location, url);
}