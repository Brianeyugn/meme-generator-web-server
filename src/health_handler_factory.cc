#include "request_factory.h"

#include "logging.h"

HealthRequestHandlerFactory::HealthRequestHandlerFactory() {
  Logger *log = Logger::GetLogger();
  log->LogDebug("HealthRequestHandlerFactory :: HealthRequestHandlerFactory: in constructor");
};

HealthRequestHandler* HealthRequestHandlerFactory::create(std::string location, NginxConfig* conf) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("HealthRequestHandlerFactory :: create: creating new HealthRequestHandler");
  return new HealthRequestHandler(location, conf);
}
