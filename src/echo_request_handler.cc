#include "echo_request_handler.h"

#include <boost/lexical_cast.hpp>

#include "logging.h"
#include "request_handler.h"

EchoRequestHandler::EchoRequestHandler()
	: RequestHandler() {
  Logger *log = Logger::GetLogger();
  log->LogDebug("In EchoRequestHandler constructor");
}

int EchoRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger *log = Logger::GetLogger();

  if (req.method_string() == "") {
    log->LogError("EchoRequestHandler: handle_request: missing HTTP method");
    return handle_bad_request(res);
  }

  log->LogInfo("EchoRequestHandler: handle_request: echoing incoming request");

  res.version(req.version());
  res.reason("OK");
  res.result(HTTP_STATUS_OK);
  res.set(http::field::content_type, "text/plain");

  std::string const string_headers = boost::lexical_cast<std::string>(req.base());
  std::string body;
  body += string_headers;
  body += req.body();
  res.body() = body;

  res.set(http::field::content_length, std::to_string(res.body().size()));

  return HTTP_STATUS_OK;
}
