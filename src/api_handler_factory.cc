#include "request_factory.h"

ApiRequestHandlerFactory::ApiRequestHandlerFactory() {};

ApiRequestHandler* ApiRequestHandlerFactory::create(std::string location, NginxConfig* conf) {
    return new ApiRequestHandler(location, conf);
}
