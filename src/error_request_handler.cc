#include "error_request_handler.h"
#include "request_handler.h"
#include "logging.h"

ErrorRequestHandler::ErrorRequestHandler()
    : RequestHandler() {
    Logger* log = Logger::GetLogger();
    log->LogInfo("ErrorRequestHandler constructor\n");
}

int ErrorRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
    return handle_not_found(res);
}
