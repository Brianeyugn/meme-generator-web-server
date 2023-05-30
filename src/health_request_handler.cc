#include "health_request_handler.h"

#include <iostream>

#include <boost/lexical_cast.hpp>

#include "logging.h"
#include "mime.h"
#include "request_handler.h"

HealthRequestHandler::HealthRequestHandler(const std::string& path, NginxConfig* config)
  : RequestHandler(), location_(path) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("In HealthRequestHandler constructor");
}

int HealthRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  res.version(req.version());

  if (req.method_string() == "") {
    log->LogError("HealthRequestHandler: handle_request: missing HTTP method");
    return handle_bad_request(res);
  }

  if (req.method() == http::verb::get) {
    log->LogInfo("HealthRequestHandler: handle_request: responding to health check");

    res.reason("OK");
    res.result(HTTP_STATUS_OK);
    res.set(http::field::content_type, "text/plain");

    std::string const string_headers = boost::lexical_cast<std::string>(req.base());
    std::string body;
    body += string_headers;
    body += req.body();
    res.body() = "OK";
    res.set(http::field::content_length, std::to_string(res.body().size()));
    return HTTP_STATUS_OK;
  }
  
  log->LogError("HealthRequestHandler: handle_request: wrong HTTP method");
  return handle_bad_request(res);
}