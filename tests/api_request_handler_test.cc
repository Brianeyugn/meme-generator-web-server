#include "api_request_handler.h"
#include "config_parser.h"
#include "request_handler.h"
#include <string>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "gtest/gtest.h"

class ApiRequestHandlerTest : public ::testing::Test {
    protected:
        NginxConfigParser config_parser;
        NginxConfig config;
        std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap;

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
            if(method == "POST") {
                req.method(http::verb::post);
            }
            if(method == "DELETE") {
                req.method(http::verb::delete_);
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

TEST_F(ApiRequestHandlerTest, HandleRequest_PostCreate_Success) {
  // Prepare test configuration
    bool success = config_parser.Parse("test_configs/config_testing", &config);
    config.populateHandlerMap(hdlrMap);
    ApiRequestHandler handler("/api", hdlrMap["/api"].second);
    std::string method = "POST";
    std::string requestURI = "/api/create";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"Content-Type: application/json"};
    std::string body = "{\"data\": \"sample data\"}";

    http::request<http::string_body> req;
    makeRequestWithSpecifiedFields(req, method, requestURI, headers, body);

    // Perform the API request handling
    http::response<http::string_body> res;
    int result = handler.handle_request(req, res);

    // Assert that the response indicates success
    EXPECT_EQ(HTTP_STATUS_CREATED, result);
    EXPECT_EQ(11, res.version());
    EXPECT_EQ("application/json", res[boost::beast::http::field::content_type]);

    // Delete test directory
    boost::filesystem::remove_all("../api_dir/create");
}

TEST_F(ApiRequestHandlerTest, HandleRequest_Get_Success) {
    bool success = config_parser.Parse("test_configs/config_testing", &config);
    config.populateHandlerMap(hdlrMap);
    ApiRequestHandler handler("/api", hdlrMap["/api"].second);
    std::string method = "POST";
    std::string requestURI = "/api/create";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"Content-Type: application/json"};
    std::string body = "{\"data\": \"sample data\"}";

    http::request<http::string_body> req;
    makeRequestWithSpecifiedFields(req, method, requestURI, headers, body);

    // Perform the API request handling
    http::response<http::string_body> res;
    handler.handle_request(req, res);

    method = "GET";
    requestURI = "/api/create/1";
    
    makeRequestWithSpecifiedFields(req, method, requestURI, headers, body);

    int result = handler.handle_request(req, res);

    // Assert that the response indicates success
    EXPECT_EQ(HTTP_STATUS_OK, result);
    EXPECT_EQ(11, res.version());
    EXPECT_EQ("application/json", res[boost::beast::http::field::content_type]);

    // Delete test directory   
    boost::filesystem::remove_all("../api_dir/create");
}

TEST_F(ApiRequestHandlerTest, HandleRequest_PutUpdate_Success) {
  // Prepare test configuration
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(hdlrMap);
  
  ApiRequestHandler handler("/api", hdlrMap["/api"].second);
  std::string method = "POST";
  std::string requestURI = "/api/create";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {"Content-Type: application/json"};
  std::string body = "{\"data\": \"sample data\"}";

  http::request<http::string_body> req;
  makeRequestWithSpecifiedFields(req, method, requestURI, headers, body);

  method = "PUT";
  requestURI = "/api/create/1";
  body = "{\"data\": \"updated data\"}";

  makeRequestWithSpecifiedFields(req, method, requestURI, headers, body);

  // Perform the API request handling
  http::response<http::string_body> res;
  int result = handler.handle_request(req, res);

  // Assert that the response indicates success
  EXPECT_EQ(HTTP_STATUS_OK, result);
  EXPECT_EQ(11, res.version());

  // Delete created test directory
  boost::filesystem::remove_all("../api_dir/create");
}

TEST_F(ApiRequestHandlerTest, HandleRequest_DeleteRemove_Success) {
  // Prepare test configuration
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(hdlrMap);
  ApiRequestHandler handler("/api", hdlrMap["/api"].second);

  std::string method = "DELETE";
  std::string requestURI = "/api/create/1";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {"Content-Type: application/json"};
  std::string body = "{\"data\": \"sample data\"}";

  http::request<http::string_body> req;
  http::response<http::string_body> res;
  
  makeRequestWithSpecifiedFields(req, "POST", "/api/create", headers, body);
  handler.handle_request(req, res);
  makeRequestWithSpecifiedFields(req, method, requestURI, headers, body);

  // Perform the API request handling
  int result = handler.handle_request(req, res);

  // Assert that the response indicates success
  EXPECT_EQ(HTTP_STATUS_OK, result);
  EXPECT_EQ(11, res.version());
  
  // Check if file has been deleted
  bool fileExists = boost::filesystem::exists("../api_dir/create/1");
  EXPECT_FALSE(fileExists);

  // Delete created test directory
  boost::filesystem::remove_all("../api_dir/create");
}

TEST_F(ApiRequestHandlerTest, HandleRequest_GetList_Success) {
  // Prepare test configuration
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(hdlrMap);
  ApiRequestHandler handler("/api", hdlrMap["/api"].second);

  std::string method = "GET";
  std::string requestURI = "/api/create";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {"Content-Type: application/json"};
  std::string body = "{\"data\": \"sample data\"}";


  http::request<http::string_body> req;
  http::response<http::string_body> res;

  makeRequestWithSpecifiedFields(req, "POST", "/api/create", headers, body);
  handler.handle_request(req, res);
  makeRequestWithSpecifiedFields(req, "POST", "/api/create", headers, body);
  handler.handle_request(req, res);
  makeRequestWithSpecifiedFields(req, method, requestURI, headers, body);

  // Perform the API request handling
  int result = handler.handle_request(req, res);

  // Assert that the response indicates success
  EXPECT_EQ(HTTP_STATUS_OK, result);
  EXPECT_EQ(11, res.version());

  // Delete created test directory
  boost::filesystem::remove_all("../api_dir/create");
}