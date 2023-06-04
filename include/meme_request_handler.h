#ifndef GOOFYGOOGLERSSERVER_MEME_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_MEME_REQUEST_HANDLER_H_

#include "request_handler.h"

#include <string>
#include <map>

#include "config_parser.h"

class MemeRequestHandler : public RequestHandler {
 public:
  MemeRequestHandler(const std::string& path, NginxConfig* config); 
  int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res);
  
 private:
  int handle_form_request(http::request<http::string_body> req, http::response<http::string_body>& res);
  int handle_create(http::request<http::string_body> req, http::response<http::string_body>& res);

  std::string location_; // Config-specified location
  std::string memes_created_root_; // Config-specified root
  std::string images_root_; // Config-specified root
  std::string html_root_; // Config-specified root

  bool bad_;

  std::string database_; // SQL database for storing memes
  std::map<int, std::string> image_map_; // Maps id to image_root_'s image
};

#endif // GOOFYGOOGLERSSERVER_MEME_REQUEST_HANDLER_H_