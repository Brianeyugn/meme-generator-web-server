#include "request_factory.h"

StaticRequestHandlerFactory::StaticRequestHandlerFactory() {
};

StaticRequestHandler* StaticRequestHandlerFactory::create(std::string location, NginxConfig* conf) {
    return new StaticRequestHandler(location, conf);
}