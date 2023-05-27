#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace beast = boost::beast;
namespace http = beast::http;

class httpParser {
  public:
    httpParser();
    std::string getResponse(http::response<http::string_body> response);
    void getFields(std::string request_string, http::request<http::string_body>& req);
};

#endif