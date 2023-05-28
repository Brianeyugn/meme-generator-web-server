#ifndef GOOFYGOOGLERSSERVER_API_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_API_REQUEST_HANDLER_H_

#include <map>
#include <string>

#include "config_parser.h"
#include "request_handler.h"

struct JSONStruct {
  std::string name;
  std::string json_data;
  int id;
};

class ApiRequestHandler : public RequestHandler {
 public:
  ApiRequestHandler(const std::string& path, NginxConfig* conf);
  int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res);

 private:
  int handle_create(const http::request<http::string_body>& req, http::response<http::string_body>& res);
  int handle_retrieve(const http::request<http::string_body>& req, http::response<http::string_body>& res);
  int handle_update(const http::request<http::string_body>& req, http::response<http::string_body>& res);
  int handle_delete(const http::request<http::string_body>& req, http::response<http::string_body>& res);
  int handle_list(const http::request<http::string_body>& req, http::response<http::string_body>& res);

  int get_json_id(const std::string& uri);
  std::string get_json_name_no_id(const std::string& uri);
  std::string get_json_name(const std::string& uri);

  std::string location_;
  std::string root_;

  bool bad_;

  static std::unordered_map<std::string, std::unordered_map<int, JSONStruct>> json_storage_;
};

static inline bool is_num(std::string id) {
  try {
    return std::to_string(std::stoi(id)) == id;
  }
  catch (std::exception& e) {
    return false;
  }
}

#endif // GOOFYGOOGLERSSERVER_API_REQUEST_HANDLER_H_
