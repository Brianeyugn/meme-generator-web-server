#include "static_request_handler.h"
#include "request_handler.h"
#include "config_parser.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include "gtest/gtest.h"

class StaticRequestHandlerTest : public ::testing::Test {

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

TEST_F(StaticRequestHandlerTest, BadMethodRequestGets400ErrorResponse) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1/text", handler_map["/static1/text"].second);
  std::string method = "LALALA"; 
  std::string requestURI = "/static1/text";
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

TEST_F(StaticRequestHandlerTest, GetRequestForHtmlFile) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1/text", handler_map["/static1/text"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/text/test.html";
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
  EXPECT_EQ(res.body(), "<!DOCTYPE html>\n<html>\n<body>\n<h1>My Heading</h1>\n<p>My paragraph</p>\n</body>\n</html>\n");
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  EXPECT_EQ(head, "Content-Type: text/html\r\nContent-Length: 86\r\n\r\n");
}

TEST_F(StaticRequestHandlerTest, GetRequestForTxtFile) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1/text", handler_map["/static1/text"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/text/test.txt";
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
  EXPECT_EQ(res.body(), "It was the best of times, it was the worst of times, it was the age of wisdom, it was the age of foolishness, it was the epoch of belief, it was the epoch of incredulity, it was the season of Light, it was the season of Darkness, it was the spring of hope, it was the winter of despair, we had everything before us, we had nothing before us, we were all going direct to Heaven, we were all going direct the other way—in short, the period was so far like the present period, that some of its noisiest authorities insisted on its being received, for good or for evil, in the superlative degree of comparison only.\n");
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  EXPECT_EQ(head, "Content-Type: text/plain\r\nContent-Length: 614\r\n\r\n");
}

TEST_F(StaticRequestHandlerTest, GetRequestForJpgFile) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1/images", handler_map["/static1/images"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/images/test.jpg";
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
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  EXPECT_EQ(head, "Content-Type: image/jpeg\r\nContent-Length: 28503\r\n\r\n");
}

TEST_F(StaticRequestHandlerTest, GetRequestForPngFile) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1/images", handler_map["/static1/images"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/images/test.png";
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
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  EXPECT_EQ(head, "Content-Type: image/png\r\nContent-Length: 23466\r\n\r\n");
}

TEST_F(StaticRequestHandlerTest, GetRequestForZipFile) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1", handler_map["/static1"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/test.zip";
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
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  EXPECT_EQ(head, "Content-Type: application/zip\r\nContent-Length: 1033\r\n\r\n");
}

TEST_F(StaticRequestHandlerTest, GetRequestForFileWithNoFileExtension) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1", handler_map["/static1"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/test";
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
  EXPECT_EQ(res.body(), "hello\n");
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  EXPECT_EQ(head, "Content-Type: application/octet-stream\r\nContent-Length: 6\r\n\r\n");
}

TEST_F(StaticRequestHandlerTest, GetRequestForNonexistentFile404Error) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1", handler_map["/static1"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/fileThatDoesNotExist";
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
  EXPECT_EQ(res.result_int(), 404);
}

TEST_F(StaticRequestHandlerTest, HandleRequestUnsupportedMethod) {
  bool success = config_parser.Parse("test_configs/config_testing", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1", handler_map["/static1"].second);
  std::string method = "PUT";
  std::string requestURI = "/static1/fileThatDoesNotExist";
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
  EXPECT_EQ(res.result_int(), HTTP_STATUS_OK);
}

// TODO: FIX TEST
// TEST_F(StaticRequestHandlerTest, NonGetRequestGetsAnEchoResponseSinceNotYetHandledByServer) {
//   bool success = config_parser.Parse("test_configs/config_testing", &config);
//   config.populateHandlerMap(handler_map);
//   StaticRequestHandler handler("/static1", handler_map["/static1"].second);
//   std::string method = "PUT";
//   std::string requestURI = "/static1/file";
//   std::string httpVersion = "HTTP/1.1";
//   std::vector<std::string> headers = {};
//   std::string body = "";
//   std::string reqStr = makeRequestStringWithSpecifiedFields(
//     method, requestURI, httpVersion, headers, body);
  
//   http::request<http::string_body>req;
//   makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

//   http::response<http::string_body>res;
//   handler.handle_request(req,res);

//   EXPECT_EQ(res.version(), 11);
//   EXPECT_EQ(res.result_int(), 200);
//   EXPECT_EQ(res.body(), reqStr);
// }

TEST_F(StaticRequestHandlerTest, BadConfigRootMissingResultsIn404Error) {
  bool success = config_parser.Parse("test_configs/static_handler_bad1", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1", handler_map["/static1"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/file";
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
  EXPECT_EQ(res.result_int(), 404);
}

TEST_F(StaticRequestHandlerTest, BadConfigRootUsedDifferentKeywordResultsIn404Error) {
  bool success = config_parser.Parse("test_configs/static_handler_bad2", &config);
  config.populateHandlerMap(handler_map);
  StaticRequestHandler handler("/static1", handler_map["/static1"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/file";
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
  EXPECT_EQ(res.result_int(), 404);
}