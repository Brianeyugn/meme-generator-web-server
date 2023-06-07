#include "meme_request_handler.h"

#include <string>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "gtest/gtest.h"

#include "request_handler.h"
#include "config_parser.h"

namespace fs = boost::filesystem;


#define CREATED_PATH "../tests/test_memes_dir/memes_created"
#define IMAGE_PATH "../tests/test_memes_dir/memes_images"
#define HTML_PATH "../tests/test_memes_dir/memes_html"

class MemeRequestHandlerTest : public ::testing::Test {
 public:
  MemeRequestHandlerTest() {
    // Create meme.db
    fs::path db_path(std::string(CREATED_PATH) + "/meme.db");
    fs::ofstream file(db_path);
  }

  ~MemeRequestHandlerTest() {
    // Cleanup meme.db
    fs::path db_path(std::string(CREATED_PATH) + "/meme.db");
    if (fs::exists(db_path)) {
      fs::remove(db_path);
    }
  }

 protected:
  NginxConfigParser config_parser;
  NginxConfig config;
  std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map;

  std::string makeRequestStringWithSpecifiedFields(std::string method = "GET", 
                                                    std::string requestURI = "/meme",
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
                                      std::string requestURI = "/meme",
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

TEST_F(MemeRequestHandlerTest, BadConfigNoMemesCreatedRootGets400ErrorResponse) {
  bool success = config_parser.Parse("test_configs/meme_no_memes_created_root", &config);
  config.populateHandlerMap(handler_map);
  MemeRequestHandler handler("/meme", handler_map["/meme"].second);
  std::string method = "GET"; 
  std::string requestURI = "/meme";
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

TEST_F(MemeRequestHandlerTest, BadConfigNoImagesRootGets400ErrorResponse) {
  bool success = config_parser.Parse("test_configs/meme_no_images_root", &config);
  config.populateHandlerMap(handler_map);
  MemeRequestHandler handler("/meme", handler_map["/meme"].second);
  std::string method = "GET"; 
  std::string requestURI = "/meme";
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

TEST_F(MemeRequestHandlerTest, BadConfigNoHTMLRootGets400ErrorResponse) {
  bool success = config_parser.Parse("test_configs/meme_no_html_root", &config);
  config.populateHandlerMap(handler_map);
  MemeRequestHandler handler("/meme", handler_map["/meme"].second);
  std::string method = "GET"; 
  std::string requestURI = "/meme";
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

TEST_F(MemeRequestHandlerTest, BadConfigEmptyHandlerGets400ErrorResponse) {
  bool success = config_parser.Parse("test_configs/meme_no_roots", &config);
  config.populateHandlerMap(handler_map);
  MemeRequestHandler handler("/meme", handler_map["/meme"].second);
  std::string method = "GET"; 
  std::string requestURI = "/meme";
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

TEST_F(MemeRequestHandlerTest, HandleFormRequest) {
  bool success = config_parser.Parse("test_configs/meme_good", &config);
  config.populateHandlerMap(handler_map);
  MemeRequestHandler handler("/meme", handler_map["/meme"].second);
  std::string method = "GET";
  std::string requestURI = "/meme";
  std::vector<std::string> headers = {};
  std::string body = "";
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handle_request(req,res);
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), HTTP_STATUS_OK);
}

TEST_F(MemeRequestHandlerTest, HandleCreate) {
  bool success = config_parser.Parse("test_configs/meme_good", &config);
  config.populateHandlerMap(handler_map);
  MemeRequestHandler handler("/meme", handler_map["/meme"].second);
  std::string method = "POST";
  std::string requestURI = "/meme/create";
  std::vector<std::string> headers = {};
  std::string body = "top_text=gggg&bottom_text=hhhh&image=0";

  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handle_request(req,res);
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), HTTP_STATUS_OK);
}

TEST_F(MemeRequestHandlerTest, HandleCreateEmptyBody) {
  bool success = config_parser.Parse("test_configs/meme_good", &config);
  config.populateHandlerMap(handler_map);
  MemeRequestHandler handler("/meme", handler_map["/meme"].second);
  std::string method = "POST";
  std::string requestURI = "/meme/create";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "\\";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handle_request(req,res);
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), HTTP_STATUS_BAD_REQUEST);
}

TEST_F(MemeRequestHandlerTest, HandleCreateBadBody) {
  bool success = config_parser.Parse("test_configs/meme_good", &config);
  config.populateHandlerMap(handler_map);
  MemeRequestHandler handler("/meme", handler_map["/meme"].second);
  std::string method = "POST";
  std::string requestURI = "/meme/create";
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
  EXPECT_EQ(res.result_int(), HTTP_STATUS_BAD_REQUEST);
}

TEST_F(MemeRequestHandlerTest, HandleRequestNoMethod) {
  bool success = config_parser.Parse("test_configs/meme_good", &config);
  config.populateHandlerMap(handler_map);
  MemeRequestHandler handler("/meme", handler_map["/meme"].second);
  std::string method = "";
  std::string requestURI = "/meme/create";
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
  EXPECT_EQ(res.result_int(), HTTP_STATUS_BAD_REQUEST);
}

TEST_F(MemeRequestHandlerTest, HandleRequestBadURI) {
  bool success = config_parser.Parse("test_configs/meme_good", &config);
  config.populateHandlerMap(handler_map);
  MemeRequestHandler handler("/meme", handler_map["/meme"].second);
  std::string method = "POST";
  std::string requestURI = "/bad/uri";
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
  EXPECT_EQ(res.result_int(), HTTP_STATUS_NOT_FOUND);
}
