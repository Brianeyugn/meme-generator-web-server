#include "config_parser.h"

#include "gtest/gtest.h"

class NginxConfigTestFixture : public ::testing::Test {
 protected:
  void SetUp() override {
      return;
  }
  NginxConfig config;
};

// Test for empty string if there are no statements.
TEST_F(NginxConfigTestFixture, NothingInStatement) {
  bool result = config.ToString(0) == "";
  EXPECT_TRUE(result);
}

// Test converting one token to a string.
TEST_F(NginxConfigTestFixture, SingleStatementToString) {
  std::shared_ptr<NginxConfigStatement> statement(new NginxConfigStatement);
  statement.get()->tokens_.push_back("string");
  statement.get()->child_block_ = nullptr;
  config.statements_.push_back(statement);
  bool result = config.ToString(0) == "string;\n";
  EXPECT_TRUE(result);
}

// Test converting multiple tokens to a string.
TEST_F(NginxConfigTestFixture, MultipleStatementsToString) {
  std::shared_ptr<NginxConfigStatement> statement_1(new NginxConfigStatement), 
                                        statement_2(new NginxConfigStatement);
  statement_1.get()->tokens_.push_back("string_1");
  statement_1.get()->child_block_ = nullptr;
  config.statements_.push_back(statement_1);

  statement_2.get()->tokens_.push_back("string_2");
  statement_2.get()->child_block_ = nullptr;
  config.statements_.push_back(statement_2);
  bool result = config.ToString(0) == "string_1;\nstring_2;\n";
  EXPECT_TRUE(result);
}

// Test checking that an empty config has no port.
TEST_F(NginxConfigTestFixture, NoValidPortFound) {
  bool result = config.GetPort() == -1;
  EXPECT_TRUE(result);
}

// Test finding a port correctly.
TEST_F(NginxConfigTestFixture, ValidPortFound) {
  std::shared_ptr<NginxConfigStatement> statement(new NginxConfigStatement);
  statement.get()->tokens_.push_back("port");
  statement.get()->tokens_.push_back("8080");
  statement.get()->child_block_ = nullptr;
  config.statements_.push_back(statement);
  bool result = config.GetPort() == 8080;
  EXPECT_TRUE(result);
}
// Test finding a port out of range.
TEST_F(NginxConfigTestFixture, PortOutOfRange) {
  std::shared_ptr<NginxConfigStatement> statement(new NginxConfigStatement);
  statement.get()->tokens_.push_back("port");
  statement.get()->tokens_.push_back("100000");
  statement.get()->child_block_ = nullptr;
  config.statements_.push_back(statement);
  bool result = config.GetPort() == -1;
  EXPECT_TRUE(result);
}

// Test finding a port in a child block.
TEST_F(NginxConfigTestFixture, ChildPortFound) {
  std::unique_ptr<NginxConfig> child_config(new NginxConfig);
  std::shared_ptr<NginxConfigStatement> statement(new NginxConfigStatement),
                                        child_statement(new NginxConfigStatement);
  child_statement.get()->tokens_.push_back("port");
  child_statement.get()->tokens_.push_back("8080");
  child_statement.get()->child_block_ = nullptr;
  child_config.get()->statements_.push_back(child_statement);

  statement.get()->child_block_ = std::move(child_config);
  config.statements_.push_back(statement);
  bool result = config.GetPort() == 8080;
  EXPECT_TRUE(result);
}
