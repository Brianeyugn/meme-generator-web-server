#include "static_request_handler.h"

#include <filesystem>
#include <fstream> // For ifstream.
#include <iostream>
#include <map>

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace http = boost::beast::http;
using http::string_body;

StaticRequestHandler::StaticRequestHandler(std::string handled_directory_name, std::string base_directory_path)
  : RequestHandler(handled_directory_name), base_directory_path_(base_directory_path) {}

// Helper functions for ParseRequest().
static std::string GetFilename_(std::string request_url) {
  size_t last_path_position = request_url.find_last_of("/");
  std::string request_filename = request_url.substr(last_path_position + 1);
  return request_filename;
}

static std::string GetContentType_(std::string extension) {
  std::map <std::string, std::string> extension_to_content_type{
    {".html", "text/html"},
    {".htm", "text/html"},
    {".txt", "text/plain"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png", "image/png"},
    {".bin", "application/octet-stream"},
    {"", "application/octet-stream"},
    {".zip", "application/zip"}
  };
  std::string content_type = extension_to_content_type.at(extension);
  return content_type;
}

// Attempt to find matching filename in base directory and return it via `file`.
static bool FileExists_(const std::string file_name, const std::string file_path, std::ifstream &file) {
  bool found_file = false;
  std::filesystem::path base_directory_folder = file_path;

  if (std::filesystem::exists(base_directory_folder)) {
    for (const auto& dir_file : std::filesystem::directory_iterator(base_directory_folder)) {
      if (dir_file.is_regular_file()) {
        std::string dir_file_path = dir_file.path().string();
        std::string dir_file_name = std::filesystem::path(dir_file_path).filename().string();
        if (file_name == dir_file_name) {
          file.open(file_path + "/" + dir_file_name);
            found_file = true;
            break;
        }
      }
    }
  }

  return found_file;
}

// Parse the static request and update the response string
// Static handler returns response depending on if file found.
Status StaticRequestHandler::ParseRequest(const http::request<string_body>& req, http::response<string_body>& res) {  // Overide parent ParseRequest();
  // Convert request to string format.
  std::ostringstream oss;
  oss << req;
  std::string request_string = oss.str();

  std::string request_url = GetRequestURL(request_string);
  std::string request_filename = GetFilename_(request_url);

  // Find requested file in Server directory.
  std::ifstream file;

  // Response Components.
  std::string response_content_type;
  std::string file_contents;

  // Valid file found in directory.
  if (FileExists_(request_filename, base_directory_path_, file)) {
    // 200 OK CASE
    res.result(http::status::ok); // 200 OK
    std::string extension = std::filesystem::path(request_filename).extension().string();
    response_content_type = GetContentType_(extension);
    res.set(http::field::content_type, response_content_type);

    // Read file data for response
    file_contents = std::string((std::istreambuf_iterator<char>(file)),
      std::istreambuf_iterator<char>());
  } else {
    // 404 NOT FOUND CASE
    res.result(boost::beast::http::status::not_found); // 404 Not Found.
    response_content_type = GetContentType_(".txt");
    file_contents = "404 Not Found. Error. The requested URL was not found on this server.";
  }

  // Check if connection keep alive demanded
  bool keep_alive_request_exists = req.keep_alive();
  if (keep_alive_request_exists == true) {
    res.set(http::field::connection, "keep-alive");
  } else {
    res.set(http::field::connection, "close");
  }

  // Response headers and message body.
  res.set(http::field::content_type, response_content_type);
  res.body() = file_contents;
  res.prepare_payload(); // Adjusts Content-Length and Transfer-Encoding field values based on body properties.

  // Error Status return.
  Status return_status = Status(0, "Status Message: Success");
  return return_status;
}
