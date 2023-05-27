#include "static_request_handler.h"
#include "request_handler.h"
#include "mime.h"
#include "logging.h"
#include <fstream> // For ifstream.
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

StaticRequestHandler::StaticRequestHandler(const std::string& path, NginxConfig* config)
  : RequestHandler() {
  location = path;
  if(config->statements_.size() < 1) {
    bad = true;
    return;
  }
  NginxConfigStatement* stmt = config->statements_[0].get();
  if(stmt->tokens_[0] != "root" || stmt->tokens_.size() != 2) {
    bad = true;
    return;
  }

  root = stmt->tokens_[1];;
  bad = false;
}

int StaticRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();
  res.version(req.version());

  if(req.method_string() == "") {
    res.reason("Bad Request");
    res.result(400);
    return 400;
  }

  if(bad) {
    std::string error_msg = "404 Not Found\r\n";
    log->LogWarn("Bad config thrown");
    res.reason("Not Found");
    res.result(404);
    res.set(http::field::content_type, "text/plain");
    res.set(http::field::content_length, std::to_string(error_msg.size()));
    res.body() = error_msg;
    return 404;
  }

  if(req.method() != http::verb::get) {
    res.reason("OK");
    res.result(200);
    res.set(http::field::content_type, "text/plain");

    std::string const string_headers = boost::lexical_cast<std::string>(req.base());
    std::string body;
    body += string_headers;
    body += req.body();
    res.body() = body;
    res.set(http::field::content_length, std::to_string(res.body().size()));
    return 200;
  }

  std::string request_uri(req.target().begin(), req.target().end());
  std::string fileExt = request_uri.substr(request_uri.find_last_of(".") + 1);
  std::string filePath = request_uri;

  filePath = root + request_uri.substr(location.length());
  log->LogInfo("StaticRequestHandler: Longest matched patch " + request_uri + " is " + location + "\n");
  log->LogInfo("StaticRequstHandler: File path used is " + filePath + "\n");
  std::ifstream istream(filePath, std::ios::in | std::ios::binary);

  if(!boost::filesystem::is_regular_file(filePath) || !istream.good()) {
    std::string error_msg = "404 Not Found\r\n";
    res.reason("File Not Found");
    res.result(404);
    res.set(http::field::content_type, "text/plain");
    res.set(http::field::content_length, std::to_string(error_msg.size()));
    res.body() = error_msg;
    return 404;
  }
  else {
    std::string body((std::istreambuf_iterator<char>(istream)),
                     (std::istreambuf_iterator<char>()));

    int content_length = body.length();
    Mime mime;
    std::string content_type = mime.getContentType(fileExt);

    res.set(http::field::content_type, content_type);
    res.set(http::field::content_length, std::to_string(content_length));

    res.reason("OK");
    res.result(200);
    res.body() = body;

    return 200;
  }
  res.reason("Bad Request");
  res.result(400);

  return 400;
}
