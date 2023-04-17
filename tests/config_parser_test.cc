#include "gtest/gtest.h"
#include "config_parser.h"

/*
    Grammar:
    <config> ::= <statement>*
    <statement> ::= <token>+ ";"
    | <token>+ "{" <config> "}"
*/

class NginxConfigParserTest : public ::testing::Test {
  protected:
    void SetUp() override {
      return;
    }
  NginxConfigParser parser;
  NginxConfig out_config;
};

// Parse() TESTS
// Test for whitespace expected after double quote.
TEST_F(NginxConfigParserTest, NoWhitespaceAfterDoubleQuote) {
  bool success = parser.Parse("no_whitespace_after_double_quote_config", &out_config);
  EXPECT_FALSE(success);
}

// GetPort() TESTS
// Test for simple port config.
TEST_F(NginxConfigParserTest, PortSimple) {
  parser.Parse("port_simple_config", &out_config);
  int server_port = out_config.GetPort();
  EXPECT_TRUE(server_port > 0);
}

// Test for one nest port config.
TEST_F(NginxConfigParserTest, PortNested) {
  parser.Parse("port_nested_config", &out_config);
  int server_port = out_config.GetPort();
  EXPECT_TRUE(server_port > 0);
}

// Test for multi-nest (deep) port config.
TEST_F(NginxConfigParserTest, PortDeepNested) {
  parser.Parse("port_deep_nested_config", &out_config);
  int server_port = out_config.GetPort();
  EXPECT_TRUE(server_port > 0);
}

// Test for non-occuring port config.
TEST_F(NginxConfigParserTest, PortNone) {
  parser.Parse("port_none_config", &out_config);
  int server_port = out_config.GetPort();
  EXPECT_TRUE(server_port == -1);
}
