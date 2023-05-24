#include "request_factory.h"

#include "request_handler.h"
#include "echo_request_handler.h"
#include "static_request_handler.h"
#include "api_request_handler.h"

RequestHandler* RequestHandlerFactory::create(const std::string& location, const std::string& url, const std::string& data_path, std::map<std::string, std::vector<int>>& file_to_id) {
  return nullptr;
}

StaticRequestHandler* StaticRequestHandlerFactory::create(const std::string& location, const std::string& url, const std::string& data_path, std::map<std::string, std::vector<int>>& file_to_id) {
  // Create and return a StaticRequestHandler instance
  // Pass necessary arguments to constructor
  return new StaticRequestHandler(location, url);
}

EchoRequestHandler* EchoRequestHandlerFactory::create(const std::string& location, const std::string& url, const std::string& data_path, std::map<std::string, std::vector<int>>& file_to_id) {
  // Create and return an EchoRequestHandler instance
  // Pass necessary arguments to constructor
  return new EchoRequestHandler(location);
}

ApiRequestHandler* ApiRequestHandlerFactory::create(const std::string& location, const std::string& url, const std::string& data_path, std::map<std::string, std::vector<int>>& file_to_id) {
// ApiRequestHandler* ApiRequestHandlerFactory::create(const std::string& location, const std::string& url) {
  // Create and return an ApiRequestHandler instance
  // Pass necessary arguments to constructor
  return new ApiRequestHandler(location, url, data_path, file_to_id);
  // return new ApiRequestHandler(location, url);
}