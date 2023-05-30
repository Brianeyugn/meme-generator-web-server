#include "request_factory.h"

#include "logging.h"

EchoRequestHandlerFactory::EchoRequestHandlerFactory() {
  Logger *log = Logger::GetLogger();
  log->LogDebug("In EchoRequestHandlerFactory constructor");
};

EchoRequestHandler* EchoRequestHandlerFactory::create(std::string location, NginxConfig* conf) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("EchoRequestHandlerFactory: create: creating new EchoRequestHandler");
  return new EchoRequestHandler();
}
