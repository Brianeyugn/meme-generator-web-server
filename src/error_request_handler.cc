#include "error_request_handler.h"
#include "request_handler.h"
#include "logging.h"

ErrorRequestHandler::ErrorRequestHandler()
    : RequestHandler() {
    Logger* log = Logger::GetLogger();
    log->LogInfo("ErrorRequestHandler constructor\n");
}

int ErrorRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
    std::string error_msg = "404 Not Found\r\n";

    res.reason("Not Found");
    res.result(404);
    res.version(req.version());
    res.set(http::field::content_type, "text/plain");
    res.set(http::field::content_length, std::to_string(error_msg.size()));
    res.body() = error_msg;

    return 404;
}