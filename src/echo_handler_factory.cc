#include "request_factory.h"

EchoRequestHandlerFactory::EchoRequestHandlerFactory() {};

EchoRequestHandler* EchoRequestHandlerFactory::create(std::string location, NginxConfig* conf) {
    return new EchoRequestHandler();
}
