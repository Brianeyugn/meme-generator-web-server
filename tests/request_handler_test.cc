#include "request_handler.h"

#include "gtest/gtest.h"

class RequestHandlerSubclass : public RequestHandler {
 public:
  RequestHandlerSubclass() : RequestHandler() {}
  int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) { return 0; }
  int call_handle_bad_request(http::response<http::string_body>& res) { return handle_bad_request(res); }
  int call_handle_not_found(http::response<http::string_body>& res) { return handle_not_found(res); }
  int call_handle_internal_server_error(http::response<http::string_body>& res) { return handle_internal_server_error(res); }
};

class RequestHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    return;
  }

  RequestHandlerSubclass request_handler;
  http::request<http::string_body> req;
  http::response<http::string_body> res;
};

TEST_F(RequestHandlerTest, HandleRequest) {
  int result = request_handler.handle_request(req, res);
  EXPECT_EQ(result, 0);
}

TEST_F(RequestHandlerTest, HandleBadRequest) {
  int result = request_handler.call_handle_bad_request(res);
  EXPECT_EQ(result, HTTP_STATUS_BAD_REQUEST);
}

TEST_F(RequestHandlerTest, HandleNotFound) {
  int result = request_handler.call_handle_not_found(res);
  EXPECT_EQ(result, HTTP_STATUS_NOT_FOUND);
}

TEST_F(RequestHandlerTest, HandleInternalServerError) {
  int result = request_handler.call_handle_internal_server_error(res);
  EXPECT_EQ(result, HTTP_STATUS_INTERNAL_SERVER_ERROR);
}
