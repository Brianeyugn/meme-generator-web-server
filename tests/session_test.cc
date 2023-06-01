#include "session.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "config_parser.h"

// class MockSession : public RealSession {
// public:
//   MockSession(boost::asio::io_service& io_service,
//               std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map,
//               std::map<std::string, RequestHandlerFactory*> routes)
//     : RealSession(io_service, handler_map, routes) {}

//   MOCK_METHOD(void, Start, (), (override));
//   MOCK_METHOD(boost::asio::ip::tcp::socket&, Socket, (), (override));
//   MOCK_METHOD(void, HandleRead, (const boost::system::error_code& error, size_t bytes_transferred), (override));
//   MOCK_METHOD(void, HandleWrite, (const boost::system::error_code& error), (override));
// };

class SessionTest : public ::testing::Test {
 protected:
  void SetUp() override {
    return;
  }

  boost::asio::io_service io_service;
  NginxConfig out_config;
  std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map;
  std::map<std::string, RequestHandlerFactory*> routes;

};


// Running start should successfully start a new session
// TEST_F(SessionTest, StartSuccess) {
//   RealSession s(io_service, handler_map, routes);
//   s.Start();
//   SUCCEED();
// }
