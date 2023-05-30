#include "request_factory.h"

HealthRequestHandlerFactory::HealthRequestHandlerFactory() {};

HealthRequestHandler* HealthRequestHandlerFactory::create(std::string location, NginxConfig* conf) {
  return new HealthRequestHandler(location, conf);
}
