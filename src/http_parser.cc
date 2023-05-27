#include "http_parser.h"
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <cstring> 
#include <bits/stdc++.h> 
#include <vector>
#include <string.h> 
#include <logging.h>
#include <algorithm>

httpParser::httpParser() { 
};

void httpParser::getFields(std::string request_string, boost::beast::http::request<http::string_body>& req) {
  std::string type;
  std::string header_cpy = "";
  std::string request_uri;
  std::vector<std::string> headers;
  std::string version;
  std::string body;

  bool checkSyntax = false;
  int cycle = 0;

  for(int i = 0; i < request_string.length(); i++) {
    switch(cycle) {
      case 0:
        if(request_string[i] != ' ') {
          type += request_string[i];
        }
        else {
          if (type == "GET" || type == "POST" ||
              type == "PUT" || type == "DELETE" ||
              type == "HEAD" || type == "CONNECT" ||
              type == "TRACE" || type == "OPTIONS") {
            if (type == "GET") {
              req.method(http::verb::get);
            }
            else if (type == "POST") {
              req.method(http::verb::post);
            }
            else if (type == "PUT") {
              req.method(http::verb::put);
            }
            else if (type == "DELETE") {
              req.method(http::verb::delete_);
            }
            else if (type == "HEAD") {
              req.method(http::verb::head);
            }
            else if (type == "CONNECT") {
              req.method(http::verb::connect);
            }
            else if (type == "OPTIONS") {
              req.method(http::verb::options);
            }
            else {
              req.method(http::verb::trace);
            }
            cycle++;
          }
          else {
            checkSyntax = true;
          }
        }
        break;
      case 1:
        if(request_string[i] != ' ') {
          request_uri += request_string[i];
        }
        else {
          cycle++;
        }
        break;

      case 2:
        if(request_string[i] != '\r') {
          version += request_string[i];
        }
        else {
          if(request_string[i + 1] == '\n' && i + 1 < request_string.length()) {
            cycle++;
            i++;
          }
          else {
            checkSyntax = true;
          }
        }
        break;
      
      case 3:
        if(request_string[i] != '\r') {
          header_cpy += request_string[i];
        }
        else {
          if(request_string[i + 1] == '\n' && i + 1 < request_string.length()) {
            if(header_cpy == "") {
              cycle++;
              i++;
              break;
            }
            headers.push_back(header_cpy);
            header_cpy = "";

            if(request_string[i + 2] == '\r' && request_string[i + 3] == '\n' && i + 3 < request_string.length()) {
              cycle++;
              i+=3;
            }
            else {
              i++;
            }
          }
          else {
            checkSyntax = true;
          }
        }
        break;
      case 4:
        body += request_string[i];
        break;
    }
    if (checkSyntax) {
      break;
    }
  }
  if(cycle != 4) {
    checkSyntax = true;
  }
  req.target(request_uri);

  for(const auto& header : headers) {
    std::size_t pos = header.find(':');
    if(pos != std::string::npos) {
      std::string name = header.substr(0, pos);
      std::string value = header.substr(pos + 2);
      req.set(name, value);
    }
  }

  req.body() = body;
}

std::string httpParser::getResponse(boost::beast::http::response<boost::beast::http::string_body> response) {
  std::string const string_headers = boost::lexical_cast<std::string>(response.base());
  std::string res;
  res += string_headers;
  res += response.body();
  return res;
}