#include "config_parser.h"

#include "gtest/gtest.h"

class NginxConfigStatementTestFixture : public ::testing::Test {
  protected:
    void SetUp() override {
      return;
    }
  NginxConfigStatement config_statement;
};

// ToString() TESTS
// Test for just newline if depth is zero with no tokens.
TEST_F(NginxConfigStatementTestFixture, NothingInStatement) {
  bool result = config_statement.ToString(0) == ";\n";
  EXPECT_TRUE(result);
}

// Test for whitespace for depth of 1 with no tokens.
TEST_F(NginxConfigStatementTestFixture, OneDepth) {
  bool result = config_statement.ToString(1) == "  ;\n";
  EXPECT_TRUE(result);
}

// Test for depth of 2 with 2 tokens.
TEST_F(NginxConfigStatementTestFixture, TwoTokens) {
  config_statement.tokens_.push_back("One");
  config_statement.tokens_.push_back("Two");
  bool result = config_statement.ToString(2) == "    One Two;\n";
  EXPECT_TRUE(result);
}

// Test for depth of 0 with 2 tokens.
TEST_F(NginxConfigStatementTestFixture, TwoTokensZeroDepth) {
  config_statement.tokens_.push_back("One");
  config_statement.tokens_.push_back("Two");
  bool result = config_statement.ToString(0) == "One Two;\n";
  EXPECT_TRUE(result);
}

// Test child block.
TEST_F(NginxConfigStatementTestFixture, ChildBlock) {
  std::unique_ptr<NginxConfig> child_config(new NginxConfig);
  std::shared_ptr<NginxConfigStatement> child_statement(new NginxConfigStatement);
  child_statement.get()->tokens_.push_back("Two");
  child_config.get()->statements_.push_back(child_statement);

  config_statement.tokens_.push_back("One");
  config_statement.child_block_ = std::move(child_config);

  bool result = config_statement.ToString(1) == "  One {\n    Two;\n  }\n";
  EXPECT_TRUE(result);
}
