#include "static_request_handler.h"

#include <fstream> // For ifstream.
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "request_handler.h"
#include "mime.h"
#include "logging.h"

StaticRequestHandler::StaticRequestHandler(const std::string& path, NginxConfig* config)
  : RequestHandler(), location_(path) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("In StaticRequestHandler constructor");
  if (config->statements_.size() < 1) {
    log->LogError("StaticRequestHandler constructor: location_ = " + path + " is missing statements in config");
    bad_ = true;
    return;
  }
  NginxConfigStatement* stmt = config->statements_[0].get();
  if (stmt->tokens_[0] != "root" || stmt->tokens_.size() != 2) {
    log->LogError("StaticRequestHandler constructor: location_ = " + path + "is missing 'root' in config");
    bad_ = true;
    return;
  }

  root_ = stmt->tokens_[1];;
  bad_ = false;
  log->LogInfo("StaticRequestHandler constructor: location_ = " + path + ", root = " + root_);
}

int StaticRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  res.version(req.version());

  if (req.method_string() == "") {
    log->LogError("StaticRequestHandler: handle_request: missing HTTP method");
    return handle_bad_request(res);
  }

  if (bad_) {
    log->LogError("StaticRequestHandler: handle_request: bad config given");
    return handle_not_found(res);
  }

  if (req.method() != http::verb::get) {
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

  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_ext = request_uri.substr(request_uri.find_last_of(".") + 1);
  std::string file_path = request_uri;

  file_path = root_ + request_uri.substr(location_.length());
  log->LogInfo("StaticRequestHandler: Longest matched patch " + request_uri + " is " + location_ + "\n");
  log->LogInfo("StaticRequstHandler: File path used is " + file_path + "\n");
  std::ifstream istream(file_path, std::ios::in | std::ios::binary);

  if (!boost::filesystem::is_regular_file(file_path) || !istream.good()) {
    return handle_not_found(res);
  } else {
    std::string body((std::istreambuf_iterator<char>(istream)),
                     (std::istreambuf_iterator<char>()));

    int content_length = body.length();
    Mime mime;
    std::string content_type = mime.getContentType(file_ext);

    res.set(http::field::content_type, content_type);
    res.set(http::field::content_length, std::to_string(content_length));

    res.reason("OK");
    res.result(HTTP_STATUS_OK);
    res.body() = body;

    return HTTP_STATUS_OK;
  }
  
  return handle_bad_request(res);
}
