#include "api_request_handler.h"
#include "logging.h"
#include "request_handler.h"
#include "mime.h"

#include <filesystem>
#include <fstream> // For ifstream.
#include <algorithm>
#include <cstdio>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

std::unordered_map<std::string, std::unordered_map<int, JSONStruct>> ApiRequestHandler::json_storage;

static inline bool is_num(std::string id) {
  try {
    return std::to_string(std::stoi(id)) == id;
  }
  catch (std::exception& e) {
    return false;
  }
}

ApiRequestHandler::ApiRequestHandler(const std::string& path, NginxConfig* config)
  : RequestHandler(), location(path) {
  Logger *log = Logger::GetLogger();
  if (config->statements_.size() < 1) {
    bad = true;
    return;
  }
  NginxConfigStatement* stmt = config->statements_[0].get();
  if (stmt->tokens_[0] != "root" || stmt->tokens_.size() != 2) {
    bad = true;
    return;
  }
  root = stmt->tokens_[1];
  bad = false;

  if (!boost::filesystem::exists(root)) {
    boost::filesystem::create_directories(root);
  }

  JSONStruct json;

  for (auto entry : boost::filesystem::recursive_directory_iterator(root)) {
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
    json.name = json_name.substr(root.length(), json_name.length() - root.length() - 1);

    std::stringstream data;
    std::ifstream json_file(json_path.string());
    data << json_file.rdbuf();
    json.jsonData = data.str();

    json_storage[json.name][json.id] = json;
  }
}

int ApiRequestHandler::getJsonId(const std::string& uri) {
  std::size_t start_pos = uri.find_last_of('/');
  std::string json_id = uri.substr(start_pos + 1);
  return is_num(json_id) ? std::stoi(json_id) : -1;
}

std::string ApiRequestHandler::getJsonName(const std::string& uri) {
  std::string json_id = std::to_string(getJsonId(uri));
  std::size_t start_pos = uri.find_last_of('/');
  std::string json_name = uri.substr(0, start_pos);
  return getJsonNameNoId(json_name);
}

std::string ApiRequestHandler::getJsonNameNoId(const std::string& uri) {
  int location_length = location.length();
  std::string json_name = uri.substr(location_length);
  return json_name;
}

int ApiRequestHandler::handle_create(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root + request_uri.substr(location.length());

  JSONStruct myJSON;

  std::size_t prefix_length = location.length();
  if (request_uri.substr(0, prefix_length) != location) {
    res.reason("Bad Request");
    res.result(400);
    return 400;
  }

  if (req.body().length() == 0) {
    res.reason("Bad Request");
    res.result(400);
    return 400;
  }

  myJSON.name = getJsonNameNoId(request_uri);
  myJSON.jsonData = req.body();

  std::unordered_map<int, JSONStruct> json_map = json_storage[myJSON.name];
  int max_id = 0;

  auto max_id_it = std::max_element(json_map.begin(), json_map.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

  if (max_id_it != json_map.end()) {
    max_id = max_id_it->first;
  }

  myJSON.id = max_id + 1;

  if (!boost::filesystem::exists(file_path)) {
    boost::filesystem::create_directories(file_path);
  }

  std::string file_name = file_path + '/' + std::to_string(myJSON.id);
  std::ofstream file(file_name, std::ios::binary);

  if (!file.is_open()) {
    res.reason("Internal Server Error");
    res.result(500);
    return 500;
  }

  file << req.body();
  file.close();

  json_storage[myJSON.name][myJSON.id] = myJSON;

  std::string body = "{\"id\": " + std::to_string(myJSON.id) + "}";
  int content_length = body.length();
  std::string content_type = "application/json";

  res.set(http::field::content_length, std::to_string(content_length));
  res.set(http::field::content_type, content_type);

  res.reason("OK");
  res.result(200);
  res.body() = body;

  return 200;
}

int ApiRequestHandler::handle_retrieve(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root + request_uri.substr(location.length());

  std::ifstream file(file_path);

  if (!file.good()) {
    res.reason("Not Found");
    res.result(404);
    return 404;
  }

  std::stringstream body;
  body << file.rdbuf();

  int content_length = body.str().length();
  std::string content_type = "application/json";

  res.set(http::field::content_length, std::to_string(content_length));
  res.set(http::field::content_type, content_type);

  res.reason("OK");
  res.result(200);
  res.body() = body.str();

  return 200;
}

int ApiRequestHandler::handle_update(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root + request_uri.substr(location.length());

  if (request_uri.substr(0, location.length()) != location) {
    res.reason("Bad Request");
    res.result(400);
    return 400;
  }

  if (getJsonId(request_uri) == -1) {
    res.reason("Bad Request");
    res.result(400);
    return 400;
  }

  std::string file_dir = root + getJsonName(request_uri);
  if (!boost::filesystem::exists(file_dir)) {
    boost::filesystem::create_directories(file_dir);
  }

  std::ofstream file(file_path, std::ios::trunc);

  if (!file.is_open()) {
    res.reason("Internal Server Error");
    res.result(500);
    return 500;
  }

  file << req.body();
  file.close();

  JSONStruct json;
  json.name = getJsonName(request_uri);
  json.id = getJsonId(request_uri);
  json.jsonData = req.body();

  json_storage[json.name][json.id] = json;

  res.reason("OK");
  res.result(200);

  return 200;
}

int ApiRequestHandler::handle_delete(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root + request_uri.substr(location.length());

  std::ifstream file(file_path);

  if (!file.good()) {
    res.reason("Not Found");
    res.result(404);
    return 404;
  }

  if (std::remove(file_path.c_str()) != 0) {
    res.reason("Internal Server Error");
    res.result(500);
    return 500;
  }

  std::string json_name = getJsonName(request_uri);
  int json_id = getJsonId(request_uri);

  json_storage[json_name].erase(json_id);

  res.reason("OK");
  res.result(200);

  return 200;
}

int ApiRequestHandler::handle_list(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
  std::string request_uri(req.target().begin(), req.target().end());
  std::string json_name = getJsonNameNoId(request_uri);

  if (json_storage.find(json_name) == json_storage.end()) {
    res.reason("Not Found");
    res.result(404);
    return 400;
  }

  std::unordered_map<int, JSONStruct> json_map = json_storage[json_name];
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
  res.result(200);
  res.body() = body;

  return 200;
}

int ApiRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  res.version(req.version());
  req.set(boost::beast::http::field::content_type, "application/json");

  if (req.method_string() == "") {
    res.reason("Bad Request");
    res.result(400);
    return 400;
  }

  if (bad) {
    std::string error_msg = "404 Not Found \r\n";

    int content_length = error_msg.length();
    std::string content_type = "text/plain";

    res.set(http::field::content_length, std::to_string(content_length));
    res.set(http::field::content_type, content_type);

    res.reason("Not Found");
    res.result(404);
    res.body() = error_msg;

    return 404;
  }

  boost::system::error_code ec;
  int ret_code;
  std::string request_uri(req.target().begin(), req.target().end());
  std::string file_path = root + request_uri.substr(location.length());

  switch (req.method()) {
    case http::verb::get:
      if (boost::filesystem::is_directory(file_path, ec)) {
        ret_code = handle_list(req, res);
      }
      else if (boost::filesystem::is_regular_file(file_path, ec)) {
        ret_code = handle_retrieve(req, res);
      }
      else {
        ret_code = 404;
        res.reason("Not Found");
        res.result(404);
      }
      break;
    case http::verb::post:
      ret_code = handle_create(req, res);
      break;
    case http::verb::put:
      ret_code = handle_update(req, res);
    case http::verb::delete_:
      ret_code = handle_delete(req, res);
      break;
    default:
      ret_code = 400;
      res.reason("Bad Request");
      res.result(400);
  }
  return ret_code;
}