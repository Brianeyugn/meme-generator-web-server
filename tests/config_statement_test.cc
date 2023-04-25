#include "gtest/gtest.h"
#include "config_parser.h"

class NginxConfigStatementTest : public ::testing::Test {
  protected:
    void SetUp() override {
      return;
    }
  NginxConfigStatement configStatement;
};

// ToString() TESTS
// Test for just newline if depth is zero with no tokens.
TEST_F(NginxConfigStatementTest, NothingInStatement) {
  bool result = configStatement.ToString(0) == ";\n";
  EXPECT_TRUE(result);
}

// Test for whitespace for depth of 1 with no tokens.
TEST_F(NginxConfigStatementTest, OneDepth) {
  bool result = configStatement.ToString(1) == "  ;\n";
  EXPECT_TRUE(result);
}

// Test for depth of 2 with 2 tokens.
TEST_F(NginxConfigStatementTest, TwoTokens) {
  configStatement.tokens_.push_back("One");
  configStatement.tokens_.push_back("Two");
  bool result = configStatement.ToString(2) == "    One Two;\n";
  EXPECT_TRUE(result);
}

// Test for depth of 0 with 2 tokens.
TEST_F(NginxConfigStatementTest, TwoTokensZeroDepth) {
  configStatement.tokens_.push_back("One");
  configStatement.tokens_.push_back("Two");
  bool result = configStatement.ToString(0) == "One Two;\n";
  EXPECT_TRUE(result);
}
