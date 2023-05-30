#include "server.h"

#include <map>
#include <string>

#include "gtest/gtest.h"

#include "config_parser.h"

class ServerTest : public ::testing::Test {
  protected:
    short port = 8080;
    boost::asio::io_service io_service;
    // We are using the real implementation of a session.
    // Using a mock is not desirable as the attempted
    // refactoring of the code expanded it too much.
    NginxConfigParser config_parser;
    NginxConfig config;
    std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map;
    ServerTest() {
      bool result = config_parser.Parse("new_format_config_for_testing", &config);
      config.populateHandlerMap(handler_map);
    }
};

// Successfully create a new session and start it
TEST_F(ServerTest, StartAcceptSuccess) {
  Server s(io_service, port, handler_map);
  bool result = s.StartAccept();
  EXPECT_TRUE(result);
}

