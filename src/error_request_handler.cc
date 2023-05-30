#include "error_request_handler.h"

#include "logging.h"
#include "request_handler.h"

ErrorRequestHandler::ErrorRequestHandler()
  : RequestHandler() {
  Logger *log = Logger::GetLogger();
  log->LogDebug("In ErrorRequestHandler constructor");
}

int ErrorRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger *log = Logger::GetLogger();
  log->LogInfo("ErrorRequestHandler: handle_request: file not found");
  return handle_not_found(res);
}
