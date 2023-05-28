#include "request_factory.h"

#include "logging.h"

ErrorHandlerFactory::ErrorHandlerFactory() {};

ErrorRequestHandler* ErrorHandlerFactory::create(std::string location, NginxConfig* conf) {
    return new ErrorRequestHandler();
}
