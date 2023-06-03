#include "request_factory.h"

#include "logging.h"

MemeRequestHandlerFactory::MemeRequestHandlerFactory() {
  Logger *log = Logger::GetLogger();
  log->LogDebug("In MemeRequestHandlerFactory constructor");
};

MemeRequestHandler* MemeRequestHandlerFactory::create(std::string location, NginxConfig* conf) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("MemeRequestHandlerFactory: create: creating new MemeRequestHandler");
  return new MemeRequestHandler(location, conf);
}
