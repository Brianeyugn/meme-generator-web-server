#include "static_request_handler.h"

#include <filesystem>
#include <fstream> // For ifstream.
#include <iostream>
#include <map>

using namespace std;

StaticRequestHandler::StaticRequestHandler(string request_string, string handled_directory_name, string base_directory_path)
  :RequestHandler(request_string, handled_directory_name) {
  this->base_directory_path_ = base_directory_path;
}

// Helper function for ParseRequest().
string StaticRequestHandler::GetFilename(string request_url) {
  int last_path_position = request_url.find_last_of("/");
  string request_filename = request_url.substr(last_path_position + 1);
  return request_filename;
}

// Helper function for ParseRequest().
string StaticRequestHandler::GetContentType(string extension) {
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
  string content_type = extension_to_content_type.at(extension);
  return content_type;
}

// Parse the static request and update the response string
void StaticRequestHandler::ParseRequest() {  // Overide parent ParseRequest();
  // Static handler returns response depending on if file found.

  string request_url = GetRequestURL(this->request_string_);
  string request_filename = GetFilename(request_url);

  // Find requested file in Server directory.
  bool found_file = false;
  ifstream file;

  // Attempt to find matching filename in base directory.
  // filesystem::path base_directory_folder = filesystem::current_path();
  filesystem::path base_directory_folder = base_directory_path_;
  if (filesystem::exists(base_directory_folder)) {
    for (const auto& dir_file : filesystem::directory_iterator(base_directory_folder)) {
      if (dir_file.is_regular_file()) {
        string dir_file_path = dir_file.path().string();
        string dir_file_name = filesystem::path(dir_file_path).filename().string();
        if (request_filename == dir_file_name) {
          file.open(base_directory_path_ + "/" + dir_file_name);
            found_file = true;
            break;
        }
      }
    }
  }

  //Response Components
  std::string response_status_code;
  std::string response_content_type;
  std::string file_contents;
  string response_content_length;

  if (found_file == true) { // Valid file found in directory.
    // 200 OK CASE 
    response_status_code = "200 OK";
    std::string extension = filesystem::path(request_filename).extension().string();
    response_content_type = GetContentType(extension);

    //Read file data for response
    file_contents = std::string((std::istreambuf_iterator<char>(file)),
      std::istreambuf_iterator<char>());
    response_content_length = to_string(file_contents.size());
  }
  else {
    // 404 NOT FOUND CASE
    response_status_code = "404 Not Found";
    response_content_type = GetContentType(".txt");
    file_contents = "404 Not Found. Error. The requested URL was not found on this Server.";
    response_content_length = to_string(file_contents.size());
  }

  // Response headers and message body.
  string response;
  response.append("HTTP/1.1 " + response_status_code + "\r\n");
  response.append("Content-Type: " + response_content_type + "\r\n");
  response.append("Content-Length: " + response_content_length + "\r\n");
  response.append("\r\n");
  response.append(file_contents);

  // Update response string.
  this->response_string_ = response;
}
