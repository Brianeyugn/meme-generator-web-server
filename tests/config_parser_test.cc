#include "config_parser.h"

#include "gtest/gtest.h"

/*
    Grammar:
    <config> ::= <statement>*
    <statement> ::= <token>+ ";"
    | <token>+ "{" <config> "}"
*/

class NginxConfigParserTestFixture : public ::testing::Test {
  protected:
    void SetUp() override {
      return;
    }
  NginxConfigParser parser;
  NginxConfig out_config;
};

// Parse() TESTS
// Test for whitespace expected after double quote.
TEST_F(NginxConfigParserTestFixture, NoWhitespaceAfterDoubleQuote) {
  bool result = parser.Parse("test_configs/config_no_whitespace_after_double_quote", &out_config);
  EXPECT_FALSE(result);
}

// GetPort() TESTS
// Test for simple port config.
TEST_F(NginxConfigParserTestFixture, PortSimple) {
  parser.Parse("test_configs/config_port_simple", &out_config);
  bool result = out_config.GetPort() > 0;
  EXPECT_TRUE(result);
}

// Test for one nest port config.
TEST_F(NginxConfigParserTestFixture, PortNested) {
  parser.Parse("test_configs/config_port_nested", &out_config);
  bool result = out_config.GetPort() > 0;
  EXPECT_TRUE(result);
}

// Test for multi-nest (deep) port config.
TEST_F(NginxConfigParserTestFixture, PortDeepNested) {
  parser.Parse("test_configs/config_port_deep_nested", &out_config);
  bool result = out_config.GetPort() > 0;
  EXPECT_TRUE(result);
}

// Test for non-occuring port config.
TEST_F(NginxConfigParserTestFixture, PortNone) {
  parser.Parse("test_configs/config_port_none", &out_config);
  bool result = out_config.GetPort() == -1;
  EXPECT_TRUE(result);
}

// Unit tests from assignment 1.

// Test with example config given in skeleton code.
TEST_F(NginxConfigParserTestFixture, SimpleConfigFixture) {
  bool result = parser.Parse("test_configs/config_example", &out_config);
  EXPECT_TRUE(result);
}

// Test with empty config file.
TEST_F(NginxConfigParserTestFixture, EmptyConfig) {
  bool result = parser.Parse("test_configs/config_empty", &out_config);
  EXPECT_TRUE(result);
}

// Test with missing quote in token.
TEST_F(NginxConfigParserTestFixture, MissingQuote) {
  bool result = parser.Parse("test_configs/config_missing_quote", &out_config);
  EXPECT_FALSE(result);
}

// Test with mixing double and single quote in token.
TEST_F(NginxConfigParserTestFixture, MixedQuote) {
  bool result = parser.Parse("test_configs/config_mixed_quote", &out_config);
  EXPECT_FALSE(result);
}

// Test with whitespace between token and semicolon.
TEST_F(NginxConfigParserTestFixture, WhitespaceSemicolon) {
  bool result = parser.Parse("test_configs/config_whitespace_semicolon", &out_config);
  EXPECT_TRUE(result);
}

// Test with whitespace after semicolon.
TEST_F(NginxConfigParserTestFixture, TrailingWhitespace) {
  bool result = parser.Parse("test_configs/config_trailing_whitespace", &out_config);
  EXPECT_TRUE(result);
}

// Test with brackets containing no tokens.
TEST_F(NginxConfigParserTestFixture, EmptyBrackets) {
  bool result = parser.Parse("test_configs/config_empty_brackets", &out_config);
  EXPECT_TRUE(result);
}

// Test with nested brackets.
TEST_F(NginxConfigParserTestFixture, NestedBrackets) {
  bool result = parser.Parse("test_configs/config_nested_brackets", &out_config);
  EXPECT_TRUE(result);
}

// Test with missing opening bracket.
TEST_F(NginxConfigParserTestFixture, MissingOpenBracket) {
  bool result = parser.Parse("test_configs/config_missing_open_bracket", &out_config);
  EXPECT_FALSE(result);
}

// Test with missing closing bracket.
TEST_F(NginxConfigParserTestFixture, MissingCloseBracket) {
  bool result = parser.Parse("test_configs/config_missing_close_bracket", &out_config);
  EXPECT_FALSE(result);
}

// Test statement containing more than one token.
TEST_F(NginxConfigParserTestFixture, StatementWithMultipleTokens) {
  bool result = parser.Parse("test_configs/config_statement_with_multiple_tokens", &out_config);
  EXPECT_TRUE(result);
}

// Additional tests for higher code coverage.

// Give the parser a filename that doesn't exist.
TEST_F(NginxConfigParserTestFixture, BadFile) {
  bool result = parser.Parse("bad_file_name", &out_config);
  EXPECT_FALSE(result);
}

// Test comments throughout the file.
TEST_F(NginxConfigParserTestFixture, FileWithComments) {
   bool result = parser.Parse("test_configs/config_comments", &out_config);
   EXPECT_TRUE(result);
}

// Test backslashes in strings.
TEST_F(NginxConfigParserTestFixture, StringWithBackslashes) {
  bool result = parser.Parse("test_configs/config_backslashes", &out_config);
  EXPECT_TRUE(result);
}
