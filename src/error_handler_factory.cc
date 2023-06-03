#include "request_factory.h"

#include "logging.h"

ErrorHandlerFactory::ErrorHandlerFactory() {
  Logger *log = Logger::GetLogger();
  log->LogDebug("ErrorHandlerFactory :: ErrorHandlerFactory: in constructor");
};

ErrorRequestHandler* ErrorHandlerFactory::create(std::string location, NginxConfig* conf) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("ErrorRequestHandlerFactory :: create: creating new ErrorRequestHandler");
  return new ErrorRequestHandler();
}
