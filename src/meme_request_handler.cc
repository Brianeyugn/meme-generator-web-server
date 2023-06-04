#include "meme_request_handler.h"

#include <fstream> // For ifstream.
#include <iostream>

#include <boost/filesystem.hpp>
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

  NginxConfigStatement* memes_created_stmt = config->statements_[0].get();
  if (memes_created_stmt->tokens_[0] != "memes_created_root" || memes_created_stmt->tokens_.size() != 2) {
    log->LogError("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + " is missing 'memes_created_root' in config");
    bad_ = true;
    return;
  }

  NginxConfigStatement* images_stmt = config->statements_[1].get();
  if (images_stmt->tokens_[0] != "images_root" || images_stmt->tokens_.size() != 2) {
    log->LogError("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + " is missing 'images_root' in config");
    bad_ = true;
    return;
  }

  NginxConfigStatement* form_stmt = config->statements_[2].get();
  if (form_stmt->tokens_[0] != "form_root" || form_stmt->tokens_.size() != 2) {
    log->LogError("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + " is missing 'form_root' in config");
    bad_ = true;
    return;
  }

  memes_created_root_ = memes_created_stmt->tokens_[1];
  images_root_ = images_stmt->tokens_[1];
  form_root_ = form_stmt->tokens_[1];
  bad_ = false;
  log->LogInfo("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + ", created_memes_root = " + memes_created_root_ 
                + ", images_root = " + images_root_ + ", form_root = " + form_root_);

  // Store all image_root_ images into a map with coresponding id number.
  int id = 0;
  boost::filesystem::directory_iterator end_itr;
  for (boost::filesystem::directory_iterator itr(images_root_); itr != end_itr; ++itr) {
    if (boost::filesystem::is_regular_file(itr->status()) && itr->path().filename() != ".DS_Store") {
      log->LogInfo("MemeRequestHandler :: MemeRequestHandler: image loaded into image_map_: ID: " + std::to_string(id) + " NAME: " + itr->path().filename().string());
      image_map_[id] = itr->path().filename().string();
      id++;
    }
  }
  log->LogInfo("MemeRequestHandler :: MemeRequestHandler: total images loaded into image_map_: " + std::to_string(image_map_.size()));
}

int MemeRequestHandler::handle_form_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  log->LogInfo("MemeRequestHandler :: handle_form_request: responding to request for a html meme form");
  
  std::string file_path = form_root_ + "/memes_form.html";
  std::string file_ext = file_path.substr(file_path.find_last_of(".") + 1);

  log->LogInfo("MemeRequestHandler :: handle_form_request: File path used is " + file_path + "\n");
  std::ifstream istream(file_path, std::ios::in | std::ios::binary);

  if (!boost::filesystem::is_regular_file(file_path) || !istream.good()) {
    return handle_not_found(res);
  } else {
    std::string body((std::istreambuf_iterator<char>(istream)),
                     (std::istreambuf_iterator<char>()));

    // Insert image options into html form
    std::string sel_target = "<option>Select a template...</option>\n";
    size_t pos = body.find(sel_target) + sel_target.length(); // Position after target string
    for (int i=0; i<image_map_.size(); i++) {
      std::string insert_str = "      <option value=" + std::to_string(i) + ">" + image_map_[i] + "</option>\n";
      body.insert(pos, insert_str);
      pos += insert_str.length();
    }
    log->LogInfo("MemeRequestHandler :: handle_form_request: HTML form body is:\n" + body);

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

int MemeRequestHandler::handle_create(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  // Placeholding code to respond to form submission.
  // Repalce with meme creation and display.

  log->LogInfo("MemeRequestHandler :: handle_create: responding to form submission");

  res.reason("OK");
  res.result(HTTP_STATUS_OK);
  res.set(http::field::content_type, "text/plain");

  std::string const string_headers = boost::lexical_cast<std::string>(req.base());
  std::string body;
  body += string_headers;
  body += req.body();
  res.body() = "Form Submitted\n";
  res.body() += req.body();
  res.set(http::field::content_length, std::to_string(res.body().size()));
  return HTTP_STATUS_OK;
}

int MemeRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  res.version(req.version());

  if (req.method_string() == "") {
    log->LogError("MemeRequestHandler :: handle_request: missing HTTP method");
    return handle_bad_request(res);
  }

  if (bad_) {
    log->LogError("MemeRequestHandler :: handle_request: bad config given");
    return handle_bad_request(res);
  }

  int ret_code = 0;
  std::string request_uri(req.target().begin(), req.target().end());

  if (request_uri == "/meme") {
    ret_code = handle_form_request(req, res);
  } else if (request_uri == "/meme/create") {
    ret_code = handle_create(req, res);
  } else if (request_uri == "memes/view") {
    // Handle listing of created memes
    ret_code = 0;
  } else if (request_uri == "memes/list") {
    // Handle listing of created memes
    ret_code = 0;
  } else {
    log->LogInfo("MemeRequestHandler :: handle_request: encountered request for nonexisting directory");
    ret_code = handle_not_found(res);
  }

  return ret_code;
}
