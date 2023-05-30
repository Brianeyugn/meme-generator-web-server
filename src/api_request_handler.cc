#include "api_request_handler.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream> // For ifstream.
#include <shared_mutex>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "logging.h"
#include "mime.h"
#include "request_handler.h"

// Mutex for CRUD operations on separate threads
std::shared_mutex api_lock;

std::unordered_map<std::string, std::unordered_map<int, JSONStruct>> ApiRequestHandler::json_storage_;

ApiRequestHandler::ApiRequestHandler(const std::string& path, NginxConfig* config)
  : RequestHandler(), location_(path) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("In ApiRequestHandler constructor");
  if (config->statements_.size() < 1) {
    log->LogError("ApiRequestHandler constructor: location_ = " + path + " is missing statements in config");
    bad_ = true;
    return;
  }
  NginxConfigStatement* stmt = config->statements_[0].get(); // assume first statement is root /path; statement
  if (stmt->tokens_[0] != "root" || stmt->tokens_.size() != 2) {
    log->LogError("ApiRequestHandler constructor: location_ = " + path + "is missing 'root' in config");
    bad_ = true;
    return;
  }
  root_ = stmt->tokens_[1];
  bad_ = false;
  log->LogInfo("ApiRequestHandler constructor: location_ = " + path + ", root = " + root_);

  // Create root directory if it doesn't already exist
  log->LogDebug("ApiRequestHandler constructor: locking thread for writing");
  std::unique_lock<std::shared_mutex> u_lock(api_lock);
  if (!boost::filesystem::exists(root_)) {
    log->LogInfo("ApiRequestHandler constructor: root path " + root_ + " does not exist, creating now");
    boost::filesystem::create_directories(root_);
  }

  // Read all currently existing data from `loc` into jsonStorage
  JSONStruct json;
  for (auto entry : boost::filesystem::recursive_directory_iterator(root_)) {
    if (!boost::filesystem::is_regular_file(entry)) {
      log->LogDebug("ApiRequestHandler constructor: encountered nonregular file in " + root_ + ", ignoring");
      continue;
    }

    std::string json_id = entry.path().stem().string();
    if (!is_num(json_id)) {
      log->LogDebug("ApiRequestHandler constructor: encountered file with nonnumeric name: " + json_id + ", ignoring");
      continue;
    }

    json.id = std::stoi(entry.path().stem().string());

    // Truncate path so it only contains the JSON name
    // (we cannot use the helper functions because this path uses `root` instead of `loc`)
    boost::filesystem::path json_path = entry.path();
    std::string json_name = json_path.remove_filename().string();
    json.name = json_name.substr(root_.length(), json_name.length() - root_.length() - 1);

    // Read file contents and store into struct
    std::stringstream data;
    std::ifstream json_file(json_path.string());
    data << json_file.rdbuf();
    json.json_data = data.str();

    // Insert into mapping
    log->LogInfo("ApiRequestHandler constructor: adding " + entry.path().string() + " to mapping");
    json_storage_[json.name][json.id] = json;
  }
  log->LogDebug("ApiRequestHandler constructor: unlocking thread");
}

int ApiRequestHandler::get_json_id(const std::string& uri) {
  std::size_t start_pos = uri.find_last_of('/');
  std::string json_id = uri.substr(start_pos + 1);
  return is_num(json_id) ? std::stoi(json_id) : -1;
}

std::string ApiRequestHandler::get_json_name(const std::string& uri) {
  std::string json_id = std::to_string(get_json_id(uri));
  std::size_t start_pos = uri.find_last_of('/');
  std::string json_name = uri.substr(0, start_pos);
  return get_json_name_no_id(json_name);
}

std::string ApiRequestHandler::get_json_name_no_id(const std::string& uri) {
  int location_length = location_.length();
  std::string json_name = uri.substr(location_length);
  return json_name;
}

int ApiRequestHandler::handle_create(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  Logger *log = Logger::GetLogger();
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root_ + request_uri.substr(location_.length());

  log->LogInfo("ApiRequestHandler: POST: longest matched path for " + request_uri + " is " + location_ + ", file path to be used is " + file_path);

  JSONStruct json;

  // Isolate the JSON's name
  std::size_t prefix_length = location_.length();
  if (request_uri.substr(0, prefix_length) != location_) {
    log->LogError("ApiRequestHandler: POST: " + location_ + " is not a prefix of " + request_uri);
    return handle_bad_request(res);
  }

  // Isolate the JSON's data (The body of the POST request)
  if (req.body().length() == 0) {
    log->LogError("ApiRequestHandler: POST: no body in request");
    return handle_bad_request(res);
  }

  json.name = get_json_name_no_id(request_uri);
  json.json_data = req.body();

  log->LogDebug("ApiRequestHandler: POST: locking thread for writing");
  std::unique_lock<std::shared_mutex> u_lock(api_lock);

  // Calculate what the JSON's ID will be
  std::unordered_map<int, JSONStruct> json_map = json_storage_[json.name];
  int max_id = 0;

  // Get the current maximum ID (we need to provide our own comparison operator since we're passing in a pair <int, JSONStruct>).
  auto max_id_it = std::max_element(json_map.begin(), json_map.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

  // Check that the container is not empty
  if (max_id_it != json_map.end()) {
    max_id = max_id_it->first;
  }

  json.id = max_id + 1;

  // Create URI directory if it doesn't already exist
  if (!boost::filesystem::exists(file_path)) {
    log->LogInfo("ApiRequestHandler: POST: creating directory " + file_path);
    boost::filesystem::create_directories(file_path);
  }

  // Create file in backend to store the JSON
  std::string file_name = file_path + '/' + std::to_string(json.id);
  std::ofstream file(file_name, std::ios::binary);

  // Write to file
  if (!file.is_open()) {
    log->LogError("ApiRequestHandler: POST: file " + file_name + " could not be opened");
    return handle_internal_server_error(res);
  }

  file << req.body();
  file.close();

  // Add JSON to our static mapping
  log->LogInfo("ApiRequestHandler: POST: new JSON, name: " + json.name + ", ID: " + std::to_string(json.id) + ", body: " + json.json_data);
  json_storage_[json.name][json.id] = json;

  log->LogDebug("ApiRequestHandler: POST: unlocking thread");
  u_lock.unlock();

  // Create JSON response
  std::string body = "{\"id\": " + std::to_string(json.id) + "}";
  int content_length = body.length();
  std::string content_type = "application/json";

  res.set(http::field::content_length, std::to_string(content_length));
  res.set(http::field::content_type, content_type);

  res.reason("OK");
  res.result(HTTP_STATUS_CREATED);
  res.body() = body;

  return HTTP_STATUS_CREATED;
}

int ApiRequestHandler::handle_retrieve(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  Logger *log = Logger::GetLogger();
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root_ + request_uri.substr(location_.length());

  log->LogDebug("ApiRequestHandler: GET: locking thread for reading");
  std::shared_lock<std::shared_mutex> s_lock(api_lock);
  std::ifstream file(file_path);

  // Check if the requested file exists
  if (!file.good()) {
    log->LogError("ApiRequestHandler: GET: path " + request_uri + " does not exist");
    return handle_not_found(res);
  }

  // Read and return JSON file
  std::stringstream body;
  body << file.rdbuf();
  log->LogDebug("ApiRequestHandler: POST: unlocking thread");
  s_lock.unlock();

  int content_length = body.str().length();
  std::string content_type = "application/json";

  res.set(http::field::content_length, std::to_string(content_length));
  res.set(http::field::content_type, content_type);

  res.reason("OK");
  res.result(HTTP_STATUS_OK);
  res.body() = body.str();

  return HTTP_STATUS_OK;
}

int ApiRequestHandler::handle_update(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  Logger *log = Logger::GetLogger();
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root_ + request_uri.substr(location_.length());

  // Make sure we only write to the correct directory
  if (request_uri.substr(0, location_.length()) != location_) {
    log->LogError("ApiRequestHandler: PUT: invalid file location: " + request_uri);
    return handle_bad_request(res);
  }

  // Make sure ID is only numeric
  if (get_json_id(request_uri) == -1) {
    log->LogError("ApiRequestHandler: PUT: entity ID of " + request_uri + " must be numeric");
    return handle_bad_request(res);
  }

  // Create URI directory if it doesn't already exist
  log->LogDebug("ApiRequestHandler: PUT: locking thread for writing");
  std::unique_lock<std::shared_mutex> u_lock(api_lock);
  std::string file_dir = root_ + get_json_name(request_uri);
  if (!boost::filesystem::exists(file_dir)) {
    log->LogInfo("ApiRequestHandler: PUT: creating directory " + file_dir);
    boost::filesystem::create_directories(file_dir);
  }

  // Create file for writing. Overwrite all previous information if this file already exists
  std::ofstream file(file_path, std::ios::trunc);

  // Write to file
  if (!file.is_open()) {
    log->LogError("ApiRequestHandler: PUT: file " + file_path + " could not be opened");
    return handle_internal_server_error(res);
  }

  file << req.body();
  file.close();

  // Update mapping
  JSONStruct json;
  json.name = get_json_name(request_uri);
  json.id = get_json_id(request_uri);
  json.json_data = req.body();

  json_storage_[json.name][json.id] = json;
  log->LogDebug("ApiRequestHandler: PUT: unlocking thread");
  u_lock.unlock();

  // Return successful
  res.reason("OK");
  res.result(HTTP_STATUS_OK);

  return HTTP_STATUS_OK;
}

int ApiRequestHandler::handle_delete(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  Logger *log = Logger::GetLogger();
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root_ + request_uri.substr(location_.length());

  log->LogDebug("ApiRequestHandler: DELETE: locking thread for writing");
  std::unique_lock<std::shared_mutex> u_lock(api_lock);
  std::ifstream file(file_path);

  // Check if the requested file exists
  if (!file.good()) {
    log->LogError("ApiRequestHandler: DELETE: path " + request_uri + "does not exist");
    return handle_not_found(res);
  }

  // Delete the file
  if (std::remove(file_path.c_str()) != 0) {
    log->LogError("ApiRequestHandler: DELETE: file " + request_uri + " could not be deleted");
    return handle_internal_server_error(res);
  }

  // Remove file from mapping
  std::string json_name = get_json_name(request_uri);
  int json_id = get_json_id(request_uri);

  json_storage_[json_name].erase(json_id);
  log->LogDebug("ApiRequestHandler: DELETE: unlocking thread");
  u_lock.unlock();

  // Return successful
  res.reason("OK");
  res.result(HTTP_STATUS_OK);

  return HTTP_STATUS_OK;
}

int ApiRequestHandler::handle_list(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  Logger *log = Logger::GetLogger();
  std::string request_uri(req.target().begin(), req.target().end());
  std::string json_name = get_json_name_no_id(request_uri);

  // Check if the requested path exists
  log->LogDebug("ApiRequestHandler: GET: locking thread for reading");
  std::shared_lock<std::shared_mutex> s_lock(api_lock);
  if (json_storage_.find(json_name) == json_storage_.end()) {
    log->LogError("ApiRequestHandler: GET: path " + request_uri + " does not exist");
    return handle_not_found(res);
  }

  // Get all JSON IDs related to this entity
  std::unordered_map<int, JSONStruct> json_map = json_storage_[json_name];
  std::string id_list = "[";
  for (auto pair : json_map) {
    int id = pair.first;
    id_list += std::to_string(id) + ", ";
  }

  log->LogDebug("ApiRequestHandler: GET: unlocking thread");
  s_lock.unlock();

  id_list.resize(id_list.size() - 2);  // remove trailing ', '
  id_list += "]";

  // Return list
  std::string body = id_list;

  int content_length = body.length();
  std::string content_type = "application/json";

  res.set(http::field::content_length, std::to_string(content_length));
  res.set(http::field::content_type, content_type);

  res.reason("OK");
  res.result(HTTP_STATUS_OK);
  res.body() = body;

  return HTTP_STATUS_OK;
}

int ApiRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger *log = Logger::GetLogger();

  res.version(req.version());

  // Make sure that the accepted request type is JSON
  req.set(boost::beast::http::field::content_type, "application/json");

  if (req.method_string() == "") {
    log->LogError("ApiRequestHandler: handle_request: missing HTTP method");
    return handle_bad_request(res);
  }

  if (bad_) {
    log->LogError("ApiRequestHandler: handle_request: bad config given");
    return handle_bad_request(res);
  }

  boost::system::error_code ec;
  int ret_code;
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root_ + request_uri.substr(location_.length());

  switch (req.method()) {
    case http::verb::get:
      // GET could be a 'list' or 'retrieve', depending on if the URI is a directory or file
      if (boost::filesystem::is_directory(file_path, ec)) {
        ret_code = handle_list(req, res);
      } else if (boost::filesystem::is_regular_file(file_path, ec)) {
        ret_code = handle_retrieve(req, res);
      } else {
        log->LogError("ApiRequestHandler: handle_request GET: path " + file_path + " does not exist (error: " + ec.message() + ")");
        ret_code = handle_not_found(res);
      }
      break;
    case http::verb::post:
      ret_code = handle_create(req, res);
      break;
    case http::verb::put:
      ret_code = handle_update(req, res);
      break;
    case http::verb::delete_:
      ret_code = handle_delete(req, res);
      break;
    default:
      log->LogInfo("ApiRequestHandler: handle_request encountered unsupported request");
      ret_code = handle_bad_request(res);
  }

  return ret_code;
}
