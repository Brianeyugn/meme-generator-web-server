#include "health_request_handler.h"
#include "request_handler.h"
#include "config_parser.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include "gtest/gtest.h"

class HealthRequestHandlerTest : public ::testing::Test {

    protected:
        NginxConfigParser config_parser;
        NginxConfig config;
        std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map;

        std::string makeRequestStringWithSpecifiedFields(std::string method = "GET", 
                                                         std::string requestURI = "/static1/sample.html",
                                                         std::string httpVersion = "HTTP/1.1",
                                                         std::vector<std::string> headers = {},
                                                         std::string body = "") {
            std::string httpRequestString = method + std::string(" ") + requestURI + std::string(" ") + \
                                            httpVersion + std::string("\r\n");
            for(int i = 0; i < headers.size() ; i++) {
                httpRequestString += headers[i] + "\r\n";
            }
            httpRequestString += "\r\n" + body;
            return httpRequestString;
        }

        void makeRequestWithSpecifiedFields(http::request<http::string_body>&req, std::string method = "GET", 
                                            std::string requestURI = "/echo/sample.html",
                                            std::vector<std::string> headers = {},
                                            std::string body = "") {
            if(method == "GET") {
                req.method(http::verb::get);
            }
            if(method == "PUT") {
                req.method(http::verb::put);
            }
            req.target(requestURI);
            for (const auto& header : headers)
            {
                std::size_t pos = header.find(':');
                if (pos != std::string::npos)
                {
                    std::string name = header.substr(0, pos);
                    std::string value = header.substr(pos + 2); // Skip the ':' and space after it
                    req.set(name, value);
                }
            }
            req.body() = body;
        }

        std::string makeResponseStringWithSpecifiedFields(std::string httpVer,
                                                          std::string statusPhrase,
                                                          std::vector<std::string> headers,
                                                          std::string body) {
            std::string res = "";
            res += httpVer + " " + statusPhrase + "\r\n";
            for(int i = 0; i < headers.size(); i++) {
                res += headers[i] + "\r\n";
            }
            res += "\r\n" + body;
            return res;
        }

        std::string responseToString(http::response<http::string_body>res) {
            std::string res_str;
            std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
            res_str += strHeaders;
            res_str += res.body();
            return res_str;
        }

};

TEST_F(HealthRequestHandlerTest, BadMethodGetRequestForHealthGets400ErrorResponse) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  HealthRequestHandler handler("/health", handler_map["/health"].second);
  std::string method = "ClearyNotACorrectMethodHere";
  std::string requestURI = "/health";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handle_request(req,res);

  std::string expectedRes = makeResponseStringWithSpecifiedFields(
    httpVersion, "400 Bad Request", headers, "");
  EXPECT_EQ(responseToString(res), expectedRes);
}

TEST_F(HealthRequestHandlerTest, GetRequestForHealth) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  HealthRequestHandler handler("/health", handler_map["/health"].second);
  std::string method = "GET";
  std::string requestURI = "/health";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handle_request(req,res);

  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), 200);
  EXPECT_EQ(res.body(), "OK");
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  EXPECT_EQ(head, "Content-Type: text/plain\r\nContent-Length: 2\r\n\r\n");
}

TEST_F(HealthRequestHandlerTest, HandleRequestUnsupportedMethod) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  HealthRequestHandler handler("/health", handler_map["/health"].second);
  std::string method = "PUT";
  std::string requestURI = "/health";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handle_request(req,res);

  EXPECT_EQ(res.result_int(), HTTP_STATUS_BAD_REQUEST);
}
