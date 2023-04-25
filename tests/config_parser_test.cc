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
  bool result = parser.Parse("test_configs/config_no_whitespace_after_double_quote", &out_config);
  EXPECT_FALSE(result);
}

// GetPort() TESTS
// Test for simple port config.
TEST_F(NginxConfigParserTest, PortSimple) {
  parser.Parse("test_configs/config_port_simple", &out_config);
  bool result = out_config.GetPort() > 0;
  EXPECT_TRUE(result);
}

// Test for one nest port config.
TEST_F(NginxConfigParserTest, PortNested) {
  parser.Parse("test_configs/config_port_nested", &out_config);
  bool result = out_config.GetPort() > 0;
  EXPECT_TRUE(result);
}

// Test for multi-nest (deep) port config.
TEST_F(NginxConfigParserTest, PortDeepNested) {
  parser.Parse("test_configs/config_port_deep_nested", &out_config);
  bool result = out_config.GetPort() > 0;
  EXPECT_TRUE(result);
}

// Test for non-occuring port config.
TEST_F(NginxConfigParserTest, PortNone) {
  parser.Parse("test_configs/config_port_none", &out_config);
  bool result = out_config.GetPort() == -1;
  EXPECT_TRUE(result);
}

// Unit tests from assignment 1.

// Test with example config given in skeleton code.
TEST_F(NginxConfigParserTest, SimpleConfigFixture) {
  bool result = parser.Parse("test_configs/config_example", &out_config);
  EXPECT_TRUE(result);
}

// Test with empty config file.
TEST_F(NginxConfigParserTest, EmptyConfig) {
  bool result = parser.Parse("test_configs/config_empty", &out_config);
  EXPECT_TRUE(result);
}

// Test with missing quote in token.
TEST_F(NginxConfigParserTest, MissingQuote) {
  bool result = parser.Parse("test_configs/config_missing_quote", &out_config);
  EXPECT_FALSE(result);
}

// Test with mixing double and single quote in token.
TEST_F(NginxConfigParserTest, MixedQuote) {
  bool result = parser.Parse("test_configs/config_mixed_quote", &out_config);
  EXPECT_FALSE(result);
}

// Test with whitespace between token and semicolon.
TEST_F(NginxConfigParserTest, WhitespaceSemicolon) {
  bool result = parser.Parse("test_configs/config_whitespace_semicolon", &out_config);
  EXPECT_TRUE(result);
}

// Test with whitespace after semicolon.
TEST_F(NginxConfigParserTest, TrailingWhitespace) {
  bool result = parser.Parse("test_configs/config_trailing_whitespace", &out_config);
  EXPECT_TRUE(result);
}

// Test with brackets containing no tokens.
TEST_F(NginxConfigParserTest, EmptyBrackets) {
  bool result = parser.Parse("test_configs/config_empty_brackets", &out_config);
  EXPECT_TRUE(result);
}

// Test with nested brackets.
TEST_F(NginxConfigParserTest, NestedBrackets) {
  bool result = parser.Parse("test_configs/config_nested_brackets", &out_config);
  EXPECT_TRUE(result);
}

// Test with missing opening bracket.
TEST_F(NginxConfigParserTest, MissingOpenBracket) {
  bool result = parser.Parse("test_configs/config_missing_open_bracket", &out_config);
  EXPECT_FALSE(result);
}

// Test with missing closing bracket.
TEST_F(NginxConfigParserTest, MissingCloseBracket) {
  bool result = parser.Parse("test_configs/config_missing_close_bracket", &out_config);
  EXPECT_FALSE(result);
}

// Test statement containing more than one token.
TEST_F(NginxConfigParserTest, StatementWithMultipleTokens) {
  bool result = parser.Parse("test_configs/config_statement_with_multiple_tokens", &out_config);
  EXPECT_TRUE(result);
}
