#include "meme_request_handler.h"

#include <iostream>

#include <boost/lexical_cast.hpp>

#include "logging.h"
#include "mime.h"
#include "request_handler.h"

MemeRequestHandler::MemeRequestHandler(const std::string& path, NginxConfig* config)
  : RequestHandler(), location_(path) {
  Logger *log = Logger::GetLogger();

  log->LogDebug("In MemeRequestHandler constructor");
    if (config->statements_.size() < 1) {
    log->LogError("MemeRequestHandler constructor: location_ = " + path + " is missing statements in config");
    bad_ = true;
    return;
  }

  NginxConfigStatement* memes_stmt = config->statements_[0].get();
  if (memes_stmt->tokens_[0] != "memes_root" || memes_stmt->tokens_.size() != 2) {
    log->LogError("MemeRequestHandler constructor: location_ = " + path + "is missing 'memes_root' in config");
    bad_ = true;
    return;
  }

  NginxConfigStatement* images_stmt = config->statements_[1].get();
  if (images_stmt->tokens_[0] != "images_root" || images_stmt->tokens_.size() != 2) {
    log->LogError("MemeRequestHandler constructor: location_ = " + path + "is missing 'images_root' in config");
    bad_ = true;
    return;
  }

  memes_root_ = memes_stmt->tokens_[1];
  images_root_ = images_stmt->tokens_[1];
  bad_ = false;
  log->LogInfo("MemeRequestHandler constructor: location_ = " + path + ", memes_root = " + memes_root_ + ", images_root = " + images_root_);
}

int MemeRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  res.version(req.version());

  if (req.method_string() == "") {
    log->LogError("MemeRequestHandler: handle_form_request: missing HTTP method");
    return handle_bad_request(res);
  }

  if (req.method() == http::verb::get) {
    log->LogInfo("MemeRequestHandler: handle_form_request: responding to meme form request");

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
  
  log->LogError("MemeRequestHandler: handle_request: wrong HTTP method");
  return handle_bad_request(res);
}