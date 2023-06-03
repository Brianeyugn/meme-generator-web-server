#include "meme_request_handler.h"

#include <iostream>

#include <boost/lexical_cast.hpp>

#include "logging.h"
#include "mime.h"
#include "request_handler.h"

MemeRequestHandler::MemeRequestHandler(const std::string& path, NginxConfig* config)
  : RequestHandler(), location_(path) {
  Logger *log = Logger::GetLogger();

  log->LogDebug("MemeRequestHandler :: MemeRequestHandler: in constructor");
    if (config->statements_.size() < 1) {
    log->LogError("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + " is missing statements in config");
    bad_ = true;
    return;
  }

  NginxConfigStatement* memes_stmt = config->statements_[0].get();
  if (memes_stmt->tokens_[0] != "memes_root" || memes_stmt->tokens_.size() != 2) {
    log->LogError("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + "is missing 'memes_root' in config");
    bad_ = true;
    return;
  }

  NginxConfigStatement* images_stmt = config->statements_[1].get();
  if (images_stmt->tokens_[0] != "images_root" || images_stmt->tokens_.size() != 2) {
    log->LogError("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + "is missing 'images_root' in config");
    bad_ = true;
    return;
  }

  memes_root_ = memes_stmt->tokens_[1];
  images_root_ = images_stmt->tokens_[1];
  bad_ = false;
  log->LogInfo("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + ", memes_root = " + memes_root_ + ", images_root = " + images_root_);
}

int MemeRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  res.version(req.version());

  if (bad_) {
    log->LogError("ApiRequestHandler :: handle_request: bad config given");
    return handle_bad_request(res);
  }

  int ret_code = 0;
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = images_root_ + request_uri.substr(location_.length());

  std::cout << request_uri << std::endl;
  std::cout << file_path << std::endl;



  return ret_code;
}
