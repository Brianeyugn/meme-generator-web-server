#include "api_request_handler.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream> // For ifstream.

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "logging.h"
#include "mime.h"
#include "request_handler.h"

std::unordered_map<std::string, std::unordered_map<int, JSONStruct>> ApiRequestHandler::json_storage_;

ApiRequestHandler::ApiRequestHandler(const std::string& path, NginxConfig* config)
  : RequestHandler(), location_(path) {
  Logger *log = Logger::GetLogger();
  if (config->statements_.size() < 1) {
    bad_ = true;
    return;
  }
  NginxConfigStatement* stmt = config->statements_[0].get();
  if (stmt->tokens_[0] != "root" || stmt->tokens_.size() != 2) {
    bad_ = true;
    return;
  }
  root_ = stmt->tokens_[1];
  bad_ = false;

  if (!boost::filesystem::exists(root_)) {
    boost::filesystem::create_directories(root_);
  }

  JSONStruct json;

  for (auto entry : boost::filesystem::recursive_directory_iterator(root_)) {
    if (!boost::filesystem::is_regular_file(entry)) {
      continue;
    }

    std::string json_id = entry.path().stem().string();
    if (!is_num(json_id)) {
      continue;
    }

    json.id = std::stoi(entry.path().stem().string());

    boost::filesystem::path json_path = entry.path();
    std::string json_name = json_path.remove_filename().string();
    json.name = json_name.substr(root_.length(), json_name.length() - root_.length() - 1);

    std::stringstream data;
    std::ifstream json_file(json_path.string());
    data << json_file.rdbuf();
    json.json_data = data.str();

    json_storage_[json.name][json.id] = json;
  }
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
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root_ + request_uri.substr(location_.length());

  JSONStruct json;

  std::size_t prefix_length = location_.length();
  if (request_uri.substr(0, prefix_length) != location_) {
    return handle_bad_request(res);
  }

  if (req.body().length() == 0) {
    return handle_bad_request(res);
  }

  json.name = get_json_name_no_id(request_uri);
  json.json_data = req.body();

  std::unordered_map<int, JSONStruct> json_map = json_storage_[json.name];
  int max_id = 0;

  auto max_id_it = std::max_element(json_map.begin(), json_map.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

  if (max_id_it != json_map.end()) {
    max_id = max_id_it->first;
  }

  json.id = max_id + 1;

  if (!boost::filesystem::exists(file_path)) {
    boost::filesystem::create_directories(file_path);
  }

  std::string file_name = file_path + '/' + std::to_string(json.id);
  std::ofstream file(file_name, std::ios::binary);

  if (!file.is_open()) {
    return handle_internal_server_error(res);
  }

  file << req.body();
  file.close();

  json_storage_[json.name][json.id] = json;

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
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root_ + request_uri.substr(location_.length());

  std::ifstream file(file_path);

  if (!file.good()) {
    return handle_not_found(res);
  }

  std::stringstream body;
  body << file.rdbuf();

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
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root_ + request_uri.substr(location_.length());

  if (request_uri.substr(0, location_.length()) != location_) {
    return handle_bad_request(res);
  }

  if (get_json_id(request_uri) == -1) {
    return handle_bad_request(res);
  }

  std::string file_dir = root_ + get_json_name(request_uri);
  if (!boost::filesystem::exists(file_dir)) {
    boost::filesystem::create_directories(file_dir);
  }

  std::ofstream file(file_path, std::ios::trunc);

  if (!file.is_open()) {
    return handle_internal_server_error(res);
  }

  file << req.body();
  file.close();

  JSONStruct json;
  json.name = get_json_name(request_uri);
  json.id = get_json_id(request_uri);
  json.json_data = req.body();

  json_storage_[json.name][json.id] = json;

  res.reason("OK");
  res.result(HTTP_STATUS_OK);

  return HTTP_STATUS_OK;
}

int ApiRequestHandler::handle_delete(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root_ + request_uri.substr(location_.length());

  std::ifstream file(file_path);

  if (!file.good()) {
    return handle_not_found(res);
  }

  if (std::remove(file_path.c_str()) != 0) {
    return handle_internal_server_error(res);
  }

  std::string json_name = get_json_name(request_uri);
  int json_id = get_json_id(request_uri);

  json_storage_[json_name].erase(json_id);

  res.reason("OK");
  res.result(HTTP_STATUS_OK);

  return HTTP_STATUS_OK;
}

int ApiRequestHandler::handle_list(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  std::string request_uri(req.target().begin(), req.target().end());
  std::string json_name = get_json_name_no_id(request_uri);

  if (json_storage_.find(json_name) == json_storage_.end()) {
    return handle_not_found(res);
  }

  std::unordered_map<int, JSONStruct> json_map = json_storage_[json_name];
  std::string id_list = "[";
  for (auto pair : json_map) {
    int id = pair.first;
    id_list += std::to_string(id) + ", ";
  }

  id_list.resize(id_list.size() - 2);
  id_list += "]";

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
  res.version(req.version());
  req.set(boost::beast::http::field::content_type, "application/json");

  if (req.method_string() == "") {
    return handle_bad_request(res);
  }

  if (bad_) {
    return handle_not_found(res);
  }

  boost::system::error_code ec;
  int ret_code;
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root_ + request_uri.substr(location_.length());

  switch (req.method()) {
    case http::verb::get:
      if (boost::filesystem::is_directory(file_path, ec)) {
        ret_code = handle_list(req, res);
      } else if (boost::filesystem::is_regular_file(file_path, ec)) {
        ret_code = handle_retrieve(req, res);
      } else {
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
      ret_code = handle_bad_request(res);
  }
  return ret_code;
}
