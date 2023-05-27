#ifndef GOOFYGOOGLERSSERVER_API_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_API_REQUEST_HANDLER_H_

#include <string>
#include <map>

#include "request_handler.h"
#include "config_parser.h"

struct JSONStruct {
  std::string name;
  std::string jsonData;
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

    int getJsonId(const std::string& uri);
    std::string getJsonNameNoId(const std::string& uri);
    std::string getJsonName(const std::string& uri);

    std::string location;
    std::string root;

    bool bad;

    static std::unordered_map<std::string, std::unordered_map<int, JSONStruct>> json_storage;
};

#endif // GOOFYGOOGLERSSERVER_API_REQUEST_HANDLER_H_
