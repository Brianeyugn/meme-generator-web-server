#include "api_request_handler.h"
#include "logging.h"

#include <filesystem>
#include <fstream> // For ifstream.
#include <iostream>
#include <stdio.h>
#include <map>

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace http = boost::beast::http;
using http::string_body;  

ApiRequestHandler::ApiRequestHandler(std::string handled_directory_name, std::string base_directory_path,  std::string data_path, std::map<std::string, std::vector<int>>& file_to_id)
  : RequestHandler(handled_directory_name), base_directory_path_(base_directory_path), data_path_(data_path), file_to_id_(file_to_id) {
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

static std::string GetPrefix_(std::string request_url) {
  size_t first_path_position = request_url.find_first_of("/");
  std::string without_first_slash = request_url.substr(first_path_position + 1);
  size_t second_path_position = without_first_slash.find_first_of("/");
  std::string request_prefix = without_first_slash.substr(0, second_path_position);
  return request_prefix;
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

  // Example: POST /api/Shoes HTTP/1.1
  log->LogDebug("Request string: " + request_string);

  // Example: POST
  std::string request_method = GetMethod_(request_string);
  log->LogDebug("Request method: " + request_method);

  // Example: /api/Shoes
  std::string request_url = GetURL_(request_string);
  log->LogDebug("Request URL: " + request_url);

  // Example: Shoes
  std::string request_entity = GetEntity_(request_url);
  log->LogDebug("Request entity: " + request_entity);

  // Example: api
  std::string request_prefix = GetPrefix_(request_url);
  log->LogDebug("Request prefix: " + request_prefix);

  int req_method_int = 0;
  if (request_method == "GET") {
    req_method_int = 1;
  } else if (request_method == "POST") {
    req_method_int = 2;
  } else if (request_method == "PUT") {
    req_method_int = 3;
  } else if (request_method == "DELETE") {
    req_method_int = 4;
  }

  switch (req_method_int) {
    // TODO: do proper checking and ensure requests are well-formed
    case 1: { // GET
      log->LogDebug("Received GET request");
      std::string prefix = "/" + request_prefix + "/";
      std::string target = request_entity;
      std::string path = data_path_ + "/" + request_entity;
      if (!(boost::filesystem::exists(path))) { // GET failure due to path not found
        res.result(http::status::not_found);
        res.set(http::field::content_type, "text/plain");
        res.body() = "No file or directory found at " + target + "." + "\n";
        res.prepare_payload();
        log->LogError("CRUD GET request: File or directory " + path + " not found.");
        // return false;
        Status return_status = Status(1, "Status Message: Fail");
        return return_status;
      }
      if (boost::filesystem::is_directory(path)) { // GET success
        log->LogError("CRUD GET request: path is found");
        std::size_t found = target.find_last_of("/");
        std::string key = target.substr(0,found);
        // std::string key = request_entity;
        std::vector<int> ids = file_to_id_[key];
        std::vector<std::string> ids_str;
        // Convert ID vector of type int to type string
        std::transform(ids.begin(), ids.end(), std::back_inserter(ids_str),
            [](const int& id) { return std::to_string(id); });
        std::string id_list = boost::algorithm::join(ids_str, ",");
        std::string formatted_list = "[" + id_list + "]\n";
        res.result(http::status::ok);
        res.set(http::field::content_type, "text/plain");
        res.body() = "ID's at " + key + ": " + formatted_list + "\n";
        res.prepare_payload();
        log->LogInfo("Sending list of IDs for GET request at " + path);
        // return false;
        Status return_status = Status(0, "Status Message: Success");
        return return_status;
      } else { // GET success
        std::ifstream file(path, std::ios::binary);
        // read from file
        if (file.good()) {
          log->LogInfo("Reading data...\n");
          file.seekg(0, std::ios::end);
          std::string content;
          content.resize(file.tellg());
          file.seekg(0, std::ios::beg);
          file.read(&content[0], content.size());
          content += "\n";

          // if found the file, set response message
          res.result(http::status::ok);
          res.body() = content;
          res.prepare_payload();
          log->LogInfo("CRUD GET request: Finish Setting Response\n");

          file.close();
          // return true;
          Status return_status = Status(0, "Status Message: Success");
          return return_status;
        }
      }
    } 
    case 2: { // POST
      log->LogDebug("Received POST request");
      std::string prefix = "/" + request_prefix + "/";
      size_t pos = req.target().find(prefix);
      // std::ostringstream oss;
      // oss << pos;
      // log->LogDebug("Req Target: " + std::string{req.target()});
      // log->LogDebug("Prefix: " + prefix);
      // log->LogDebug("Pos: " + oss.str());
      //if the given location is found, url starts with crud location,
      //and the string of location is smaller than the url, this is a valid path
      if (pos != std::string::npos && pos == 0 && req.target().length() > prefix.length()) {
        log->LogDebug("CRUD POST request: path is valid");
        std::string key = request_entity;
        std::string path = data_path_ + "/" + request_entity;
        int value;
        //remove trailing slash
        while(path.length() >= 1 && path[path.length()-1]=='/') {
          path.pop_back();
        }
        //if entity does not exist yet
        if (file_to_id_.find(key) == file_to_id_.end()) {
          log->LogDebug("CRUD POST request: entity doesn't exist");
          try {
            boost::filesystem::create_directory(path);
          }
          catch (const boost::filesystem::filesystem_error& e) { // POST success
            log->LogInfo("Unable to create directory for " + key + "\n");
            //unprocessable entity
            res.result(http::status::unprocessable_entity);
            res.set(http::field::content_type, "text/html");
            res.body() = "<html><head><title>Unprocessable Entity</title></head><body><h1>422 Unprocessable Entity</h1></body></html>";
            res.prepare_payload();
            // return false;
            Status return_status = Status(0, "Status Message: Success");
            return return_status;
          }
          std::vector<int> values = {1};
          file_to_id_[key] = values;
          value = 1;
          log->LogInfo("Created directory for new entity: " + key + "\n");
        } else { // if entity already exists
          log->LogDebug("CRUD POST request: entity already exists");
          int lowest_available_id = 1;
          //finds the lowest available id
          for (int i = 0; i < file_to_id_[key].size(); i++) {
            if (file_to_id_[key][i] == lowest_available_id) {
              lowest_available_id++;
            }
          }
          file_to_id_[key].push_back(lowest_available_id);
          std::vector<int> values_copy = file_to_id_[key];
          std::sort(values_copy.begin(), values_copy.end());
          file_to_id_[key] = values_copy;
          value = lowest_available_id;
        }
        std::ostringstream oss;
        oss << req.body();
        std::string body = oss.str();
        oss.clear();
        std::ofstream file(path + "/" + std::to_string(value));
        file << body;
        file.close();
        log->LogInfo("Created id " + std::to_string(value) + " for entity " + key + "\n");
        //prepare response
        res.result(http::status::created);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Created entry at {\"id\":" + std::to_string(value) + "}" + "\n";
        res.prepare_payload();
        // return true;
        Status return_status = Status(0, "Status Message: Success");
        return return_status;
      }
      //cannot find the folder
      res.result(http::status::not_found);
      res.set(http::field::content_type, "text/html");
      res.body() = "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";;
      res.prepare_payload();
      Status return_status = Status(0, "Status Message: Success");
      return return_status;
    } 
    case 3: { // PUT
      log->LogDebug("Received PUT request");
      std::string prefix = "/" + request_prefix + "/";
      size_t pos = req.target().find(prefix);
      //if the given location is found, url starts with crud location,
      //and the string of location is smaller than the url, this is a valid path
      if (pos != std::string::npos && pos == 0 && req.target().length() > prefix.length()) {
        std::string key = std::string(req.target().substr(prefix.length()));
        int value;
        //remove trailing slash
        while(key.length() >= 1 && key[key.length()-1] == '/') {
          key.pop_back();
        }
        std::size_t found = key.find_last_of("/");
        std::string id_name = key.substr(found+1);
        std::string file_name = key.substr(0, found);
        std::string::const_iterator it = id_name.begin();
        while (it != id_name.end() && std::isdigit(*it)) {
          ++it;
        }
        //if after last / is integer, this is a valid path
        if(!id_name.empty() && it == id_name.end()) {
          //if key before last / exists
          value = std::stoi(id_name);
          if (file_to_id_.find(file_name) != file_to_id_.end()) {
            std::vector<int> id_names = file_to_id_[file_name]; 
            if (std::find(id_names.begin(), id_names.end(), value) == id_names.end()) {
              id_names.push_back(value);
              std::vector<int> values_copy = id_names;
              std::sort(values_copy.begin(), values_copy.end());
              file_to_id_[file_name] = values_copy;
              res.result(http::status::created);
              res.body() = "Created entry at {\"id\":" + std::to_string(value) + "}" + "\n";
              log->LogInfo("Created id " + std::to_string(value) + " for entity " + file_name + "\n");
              } else {
                res.result(http::status::ok);
                res.body() = "Updated entry at {\"id\":" + std::to_string(value) + "}" + "\n";
                log->LogInfo("File updated for id " + std::to_string(value) + " for entity " + file_name + "\n");
              }
          }
          //if key before last / does not exist
          else {
            try {
              boost::filesystem::create_directory(GetURL_(request_string) + '/' + file_name);
            } 
            catch (const boost::filesystem::filesystem_error& e) {
              log->LogInfo("Unable to create directory for " + key + "\n");
              //unprocessable entity
              res.result(http::status::unprocessable_entity);
              res.set(http::field::content_type, "text/html");
              res.body() = "<html><head><title>Unprocessable Entity</title></head><body><h1>422 Unprocessable Entity</h1></body></html>";
              res.prepare_payload();
              // return false;
              Status return_status = Status(1, "Status Message: Bad Request");
              return return_status;
            }
            std::vector<int> values = {value};
            file_to_id_[file_name] = values;
            res.result(http::status::created);
            res.body() = "Created entry at {\"id\":" + std::to_string(value) + "}" + "\n";
            log->LogInfo("Created directory for new entity: " + file_name + " and created id " + std::to_string(value)+ "\n");
          }  
          std::string path = GetURL_(request_string) + "/" + key;
          std::ostringstream oss;     
          oss << req.body();
          std::string body = oss.str();
          oss.clear();
          //ofstream automatically overwrites, which is expected for PUT
          std::ofstream file(path);
          file << body;
          file.close();
          res.prepare_payload();
          //prepare response
          // return true;
          Status return_status = Status(0, "Status Message: Success");
          return return_status;
        } else {
          //else error bad request
          res.result(http::status::bad_request);
          res.set(http::field::content_type, "text/html");
          res.body() = "<html><head><title>Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>";
          res.prepare_payload();
          // return false;
          Status return_status = Status(1, "Status Message: Bad Request");
          return return_status;
        }
    }    
    // cannot find the folder
    res.result(http::status::not_found);
    res.set(http::field::content_type, "text/html");
    res.body() = "<html><head><title>Unprocessable Entity</title></head><body><h1>422 Unprocessable Entity</h1></body></html>";
    res.prepare_payload();
    // return false;
    Status return_status = Status(0, "Status Message: Success");
    return return_status;
    } 
    case 4: { // DELETE
      log->LogDebug("Received DELETE request");
      std::string prefix = "/" + request_prefix + "/";
      std::string target = request_entity;
      std::string path_str = data_path_ + "/" + request_entity;
      if (!(boost::filesystem::exists(path_str))) {
        res.result(http::status::not_found);
        res.set(http::field::content_type, "text/plain");
        res.body() = "No file found at /" + target + "." + "\n";
        res.prepare_payload();
        log->LogError("File not found.");
        Status return_status = Status(1, "Status Message: Fail");
        return return_status;
      }
      if (boost::filesystem::is_directory(path_str)) { // DELETE fail
        res.result(http::status::bad_request);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Invalid format; expected a file ID.\n";
        res.prepare_payload();
        log->LogWarn("Warning: CRUD DELETE operations must specify a file ID");
        // return false;
        Status return_status = Status(1, "Status Message: Fail");
        return return_status;
      }
      std::size_t found = target.find_last_of("/");
      std::string key = target.substr(0,found);
      std::string value_str = target.substr(found+1,target.length());
      int value;
      try {
        value = std::stoi(value_str);
      } catch (const std::exception&) {
        res.result(http::status::bad_request);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Invalid format; expected a file ID.\n";
        res.prepare_payload();
        log->LogWarn("Warning: CRUD DELETE operations must specify a file ID");
        // return false;
        Status return_status = Status(1, "Status Message: Fail");
        return return_status;
      }

      log->LogWarn("Incoming CRUD request to delete file: /" + target);
      const char* path = path_str.c_str();
      if (file_to_id_.find(key) != file_to_id_.end()) {
        if (std::remove(path) == 0) {
          // Remove value from file:id mapping
          file_to_id_[key].erase(std::remove(file_to_id_[key].begin(), file_to_id_[key].end(), value), file_to_id_[key].end());
          res.result(http::status::ok);
          res.set(http::field::content_type, "text/plain");
          res.body() = "Successfully deleted file at /" + target + "." + "\n";
          res.prepare_payload();
          log->LogInfo("Successfully deleted file at /" + target);
          Status return_status = Status(0, "Status Message: Success");
          return return_status;
        } else {
          res.result(http::status::internal_server_error);
          res.set(http::field::content_type, "text/plain");
          res.body() = "Error while deleting file at /" + target + "." + "\n";
          log->LogError("CRUD request to delete file at /" + target + " failed;");
        }
      } else {
        res.result(http::status::internal_server_error);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Could not find ID at /" + target + "." + "\n";
        log->LogError("CRUD request to delete file at /" + target + " failed; no file ID");
      }
      res.prepare_payload();
      // return false;
      Status return_status = Status(1, "Status Message: Fail");
      return return_status;
      break;
    }
    default: {
      res.result(http::status::bad_request);
      res.set(http::field::content_type, "text/plain");
      res.body() = "Invalid CRUD method.\n";
      res.prepare_payload();
      Status return_status = Status(1, "Status Message: Fail");
      return return_status;
    }
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
