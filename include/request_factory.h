#ifndef GOOFYGOOGLERSSERVER_REQUEST_FACTORY_H_
#define GOOFYGOOGLERSSERVER_REQUEST_FACTORY_H_

#include <string>
#include <vector>
#include <boost/asio.hpp>

#include "request_handler.h"
#include "static_request_handler.h"
#include "echo_request_handler.h"
#include "api_request_handler.h"

class RequestHandlerFactory {
public:
  virtual RequestHandler* create(const std::string& location, const std::string& url, std::map<std::string, std::vector<int>>& file_to_id);
   
};

class StaticRequestHandlerFactory : public RequestHandlerFactory {
public:
  StaticRequestHandler* create(const std::string& location, const std::string& url, std::map<std::string, std::vector<int>>& file_to_id) override;
private:
};

class EchoRequestHandlerFactory : public RequestHandlerFactory {
public:
  EchoRequestHandler* create(const std::string& location, const std::string& url, std::map<std::string, std::vector<int>>& file_to_id) override;
};

class ApiRequestHandlerFactory : public RequestHandlerFactory {
public:
  ApiRequestHandler* create(const std::string& location, const std::string& url, std::map<std::string, std::vector<int>>& file_to_id) override;
  // ApiRequestHandler* create(const std::string& location, const std::string& url) override;
private:
  std::map<std::string, std::vector<int>> file_to_id;
};

#endif // GOOFYGOOGLERSSERVER_REQUEST_FACTORY_H_