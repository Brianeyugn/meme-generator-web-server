#include "request_factory.h"

#include "logging.h"

StaticRequestHandlerFactory::StaticRequestHandlerFactory() {
  Logger *log = Logger::GetLogger();
  log->LogDebug("StaticRequestHandlerFactory :: StaticRequestHandlerFactory: in constructor");
};

StaticRequestHandler* StaticRequestHandlerFactory::create(std::string location, NginxConfig* conf) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("StaticRequestHandlerFactory :: create: creating new StaticRequestHandler");
  return new StaticRequestHandler(location, conf);
}
