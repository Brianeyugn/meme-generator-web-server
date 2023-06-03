#include "request_factory.h"

#include "logging.h"

ApiRequestHandlerFactory::ApiRequestHandlerFactory() {
  Logger *log = Logger::GetLogger();
  log->LogDebug("ApiRequestHandlerFactory :: ApiRequestHandlerFactory: in constructor");
};

ApiRequestHandler* ApiRequestHandlerFactory::create(std::string location, NginxConfig* conf) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("ApiRequestHandlerFactory :: create: creating new ApiRequestHandler");
  return new ApiRequestHandler(location, conf);
}
