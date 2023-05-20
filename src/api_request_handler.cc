#include "api_request_handler.h"
#include "logging.h"

#include <filesystem>
#include <fstream> // For ifstream.
#include <iostream>
#include <map>

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace http = boost::beast::http;
using http::string_body;

ApiRequestHandler::ApiRequestHandler(std::string handled_directory_name, std::string base_directory_path)
  : RequestHandler(handled_directory_name), base_directory_path_(base_directory_path) {
    Logger *log = Logger::GetLogger();
  }

// Helper functions for ParseRequest().
static std::string GetMethod_(std::string request_string) {
  size_t first_space_position = request_string.find_first_of(" ");
  std::string request_method = request_string.substr(0, first_space_position);
  return request_method;
}

static std::string GetURL_(std::string request_string) {
  size_t first_space_position = request_string.find_first_of(" ");
  std::string without_first = request_string.substr(first_space_position + 1);
  size_t last_space_position = without_first.find_first_of(" ");
  std::string request_url = without_first.substr(0, last_space_position);
  return request_url;
}

static std::string GetEntity_(std::string request_url) {
  size_t first_path_position = request_url.find_first_of("/");
  std::string without_method = request_url.substr(first_path_position + 1);
  size_t second_path_position = without_method.find_first_of("/");
  std::string without_api = without_method.substr(second_path_position + 1);
  size_t third_path_position = without_api.find_first_of("/");
  if (third_path_position == std::string::npos) { // no "/" left
    return without_api;
  }
  std::string request_entity = without_api.substr(0, third_path_position);
  return request_entity;
}

static std::string GetID_(std::string request_url) {
  size_t last_path_position = request_url.find_last_of("/");
  std::string request_id = request_url.substr(last_path_position + 1);
  return request_id;
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

// Parse the Api request and update the response string
// Api handler returns response depending on if file found.
Status ApiRequestHandler::ParseRequest(const http::request<string_body>& req, http::response<string_body>& res) {  // Overide parent ParseRequest();
  Logger *log = Logger::GetLogger();
  log->LogDebug("Starting to parse Api request");
  // Convert request to string format.
  std::ostringstream oss;
  oss << req;
  std::string request_string = oss.str();
  log->LogDebug("Request string: " + request_string);

  std::string request_method = GetMethod_(request_string);
  log->LogDebug("Request method: " + request_method);

  std::string request_url = GetURL_(request_string);
  log->LogDebug("Request URL: " + request_url);

  std::string request_entity = GetEntity_(request_url);
  log->LogDebug("Request entity: " + request_entity);

  // TODO: do proper checking and ensure requests are well-formed
  if (request_method == "GET") { // retrieve, list
    std::string request_id = GetID_(request_url);
    log->LogDebug("Request ID: " + request_id);
    std::string action;
    int numeric_id;
    if (boost::conversion::try_lexical_convert<int>(request_id, numeric_id)) {
      action = "retrieve";
    } else {
      action = "list";
    }
    log->LogDebug("action: " + action);
  } else if (request_method == "POST") { // create
  } else if (request_method == "PUT") { // update
    std::string request_id = GetID_(request_url);
    log->LogDebug("Request ID: " + request_id);
  } else if (request_method == "DELETE") { // delete
    std::string request_id = GetID_(request_url);
    log->LogDebug("Request ID: " + request_id);
  } else {
    log->LogError("Invalid request method");
  }

  // Response Components.
  std::string response_content_type;
  std::string file_contents;

  res.result(boost::beast::http::status::not_found); // 404 Not Found.
  response_content_type = "text/plain";
  file_contents = "404 Not Found. Error. The requested URL was not found on this server.";

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
