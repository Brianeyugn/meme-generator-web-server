#include "config_parser.h"

#include "gtest/gtest.h"

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

TEST_F(NginxConfigParserTest, SimpleConfig) {
  bool success = parser.Parse("test_configs/config_example", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, SimpleConfigOutputHasCorrectState) {
  bool success = parser.Parse("test_configs/config_example", &out_config);
  EXPECT_TRUE(success);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 2);
  EXPECT_EQ(stmt0->tokens_[0], "foo");
  EXPECT_EQ(stmt0->tokens_[1], "\"bar\"");
  NginxConfigStatement *stmt1 = (statements[1].get());
  EXPECT_EQ(stmt1->tokens_[0], "server");
  NginxConfigStatement *stmt1_0 = stmt1->child_block_.get()->statements_[0].get();
  NginxConfigStatement *stmt1_1 = stmt1->child_block_.get()->statements_[1].get();
  NginxConfigStatement *stmt1_2 = stmt1->child_block_.get()->statements_[2].get();
  EXPECT_EQ(stmt1_0->tokens_[0], "port");
  EXPECT_EQ(stmt1_0->tokens_[1], "80");
  EXPECT_EQ(stmt1_1->tokens_[0], "server_name");
  EXPECT_EQ(stmt1_1->tokens_[1], "foo.com");
  EXPECT_EQ(stmt1_2->tokens_[0], "root");
  EXPECT_EQ(stmt1_2->tokens_[1], "/home/ubuntu/sites/foo/");
}

TEST_F(NginxConfigParserTest, SimpleDirectiveMissingSemicolon) {
  bool success = parser.Parse("test_configs/config_1", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, SimpleDirectiveManyParamsAndExtraSpaces) {
  bool success = parser.Parse("test_configs/config_2", &out_config);
  EXPECT_TRUE(success);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 4);
  std::cout << out_config.ToString(0) << std::endl;
}

TEST_F(NginxConfigParserTest, SimpleDirectiveOneToken) {
  bool success = parser.Parse("test_configs/config_3", &out_config);
  EXPECT_TRUE(success);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 1);
}

TEST_F(NginxConfigParserTest, BlockDirectiveEmpty) {
  bool success = parser.Parse("test_configs/config_4", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, BlockDirectiveManyParams) {
  bool success = parser.Parse("test_configs/config_5", &out_config);
  EXPECT_TRUE(success);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 4);
}

TEST_F(NginxConfigParserTest, MissingSpaceAfterDoubleQuotedToken) {
  bool success = parser.Parse("test_configs/config_6", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, MissingSpaceAfterSingleQuotedToken) {
  bool success = parser.Parse("test_configs/config_7", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, EmptyConfigFile) {
  bool success = parser.Parse("test_configs/config_8", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, BlockDirectiveWithoutTokenBeforeIt) {
  bool success = parser.Parse("test_configs/config_9", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, SimpleNestedBlockDirectives) {
  bool success = parser.Parse("test_configs/config_10", &out_config);
  EXPECT_TRUE(success);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 1);
  EXPECT_EQ(stmt0->tokens_[0], "http");
  EXPECT_TRUE(stmt0->child_block_.get() != nullptr);
  NginxConfigStatement *stmt0_0 = stmt0->child_block_.get()->statements_[0].get();
  EXPECT_EQ(stmt0_0->tokens_.size(), 1);
  EXPECT_EQ(stmt0_0->tokens_[0], "server");
}

TEST_F(NginxConfigParserTest, MismatchedBraces_ExtraOpeningCurlyBraces) {
  bool success = parser.Parse("test_configs/config_11", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, MismatchedBraces_ExtraClosingCurlyBraces) {
  bool success = parser.Parse("test_configs/config_12", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, MultipleNestedBlockDirectives) {
  bool success = parser.Parse("test_configs/config_13", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, VariousPlacementsOfComments) {
  bool success = parser.Parse("test_configs/config_14", &out_config);
  EXPECT_TRUE(success);
  std::cout << out_config.ToString(0) << std::endl;
}

TEST_F(NginxConfigParserTest, QuoteIsLastCharInFile) {
  bool success = parser.Parse("test_configs/config_15", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, MissingEndQuoteAfterString) {
  bool success = parser.Parse("test_configs/config_16", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, SemicolonAfterBlockClosingBrace) {
  bool success = parser.Parse("test_configs/config_17", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, ClosingBraceAfterNormalTokenType) {
  bool success = parser.Parse("test_configs/config_18", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, SoleSingleQuotationMark) {
  bool success = parser.Parse("test_configs/config_19", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, SoleDoubleQuotationMark) {
  bool success = parser.Parse("test_configs/config_20", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, MissingSpaceBeforeQuotedToken) {
  bool success = parser.Parse("test_configs/config_21", &out_config);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, QuotedTokenSpansMultipleLines) {
  bool success = parser.Parse("test_configs/config_22", &out_config);
  EXPECT_TRUE(success);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 2);
  EXPECT_EQ(stmt0->tokens_[0], "foo");
  EXPECT_EQ(stmt0->tokens_[1], "\"bar\n  bruin\nbear \"");
  std::cout << out_config.ToString(0) << std::endl;
}

// Curly braces serve as separators/whitespace so we don't need to require whitespace
// before and after curly braces
TEST_F(NginxConfigParserTest, NoWhitespaceBeforeAndAfterCurlyBraces) {
  bool success = parser.Parse("test_configs/config_23", &out_config);
  EXPECT_TRUE(success);
  std::cout << out_config.ToString(0) << std::endl;
}

TEST_F(NginxConfigParserTest, NoWhitespaceBetweenNestedBlocks) {
  bool success = parser.Parse("test_configs/config_24", &out_config);
  EXPECT_TRUE(success);
  std::cout << out_config.ToString(0) << std::endl;
}

TEST_F(NginxConfigParserTest, BackslashEscapeSingleQuoteWithinSingleQuotedString) {
  bool success = parser.Parse("test_configs/config_25", &out_config);
  EXPECT_TRUE(success);
  std::cout << out_config.ToString(0) << std::endl;
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 1);
  EXPECT_EQ(stmt0->tokens_[0], "\'location\'name\'");
}

TEST_F(NginxConfigParserTest, BackslashEscapeNewlineWithinSingleQuotedString) {
  bool success = parser.Parse("test_configs/config_26", &out_config);
  EXPECT_TRUE(success);
  std::cout << out_config.ToString(0) << std::endl;
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 1);
  EXPECT_EQ(stmt0->tokens_[0], "\'location\nname\'");
}

TEST_F(NginxConfigParserTest, BackslashEscapeTabWithinSingleQuotedString) {
  bool success = parser.Parse("test_configs/config_27", &out_config);
  EXPECT_TRUE(success);
  std::cout << out_config.ToString(0) << std::endl;
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 1);
  EXPECT_EQ(stmt0->tokens_[0], "\'location\tname\'");
}

TEST_F(NginxConfigParserTest, BackslashEscapeDoubleQuoteWithinDoubleQuotedString) {
  bool success = parser.Parse("test_configs/config_28", &out_config);
  EXPECT_TRUE(success);
  std::cout << out_config.ToString(0) << std::endl;
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 1);
  EXPECT_EQ(stmt0->tokens_[0], "\"location\"name\"");
}

TEST_F(NginxConfigParserTest, BackslashEscapeNewlineWithinDoubleQuotedString) {
  bool success = parser.Parse("test_configs/config_29", &out_config);
  EXPECT_TRUE(success);
  std::cout << out_config.ToString(0) << std::endl;
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 1);
  EXPECT_EQ(stmt0->tokens_[0], "\"location\nname\"");
}

TEST_F(NginxConfigParserTest, BackslashEscapeTabWithinDoubleQuotedString) {
  bool success = parser.Parse("test_configs/config_30", &out_config);
  EXPECT_TRUE(success);
  std::cout << out_config.ToString(0) << std::endl;
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  EXPECT_EQ(stmt0->tokens_.size(), 1);
  EXPECT_EQ(stmt0->tokens_[0], "\"location\tname\"");
}

TEST_F(NginxConfigParserTest, BackslashEscapeBackslashWithinQuotedStrings) {
  bool success = parser.Parse("test_configs/config_31", &out_config);
  EXPECT_TRUE(success);
  std::cout << out_config.ToString(0) << std::endl;
  std::vector<std::shared_ptr<NginxConfigStatement>> statements = out_config.statements_;
  NginxConfigStatement *stmt0 = (statements[0].get());
  NginxConfigStatement *stmt1 = (statements[1].get());
  EXPECT_EQ(stmt0->tokens_.size(), 1);
  EXPECT_EQ(stmt1->tokens_.size(), 1);
  EXPECT_EQ(stmt0->tokens_[0], "\'\\data\\images\'");
  EXPECT_EQ(stmt1->tokens_[0], "\"\\data\\images\"");
}

TEST_F(NginxConfigParserTest, GetListeningPortTest) {
  bool success = parser.Parse("test_configs/config_32", &out_config);
  EXPECT_TRUE(success);
  int port = out_config.GetPort();
  EXPECT_EQ(port, 80);
}

TEST_F(NginxConfigParserTest, ConfigPortNumOutOfRangeSoUseDefault) {
  bool success = parser.Parse("test_configs/config_33", &out_config);
  EXPECT_TRUE(success);
  int port = out_config.GetPort();
  EXPECT_EQ(port, 8080);
}

TEST_F(NginxConfigParserTest, ConfigListenKeywordMissingSoUseDefaultPort) {
  bool success = parser.Parse("test_configs/config_34", &out_config);
  EXPECT_TRUE(success);
  int port = out_config.GetPort();
  EXPECT_EQ(port, 8080);
}

TEST_F(NginxConfigParserTest, SingleQuoteInsideDoubleQuotedString) {
  bool success = parser.Parse("test_configs/config_35", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, DoubleQuoteWithinSingleQuotedString) {
  bool success = parser.Parse("test_configs/config_36", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, GetFilesystemPathsTest) {
  bool success = parser.Parse("test_configs/config_37", &out_config);
  EXPECT_TRUE(success);
  std::map<std::string, std::string> fsPaths = out_config.GetFilesystemPath();
  EXPECT_EQ(fsPaths["/static1"], "./data1");
  EXPECT_EQ(fsPaths["/static2"], "./data2");
}

TEST_F(NginxConfigParserTest, removeSlashes) {
  std::string result = out_config.removeSlash("/dir1/dir2/dir3////");
  EXPECT_EQ(result, "/dir1/dir2/dir3");
  result = out_config.removeSlash("/dir1/dir2/dir3");
  EXPECT_EQ(result, "/dir1/dir2/dir3");
  result = out_config.removeSlash("/dir1/");
  EXPECT_EQ(result, "/dir1");
  result = out_config.removeSlash("//");
  EXPECT_EQ(result, "/");
  result = out_config.removeSlash("/");
  EXPECT_EQ(result, "/");
}

TEST_F(NginxConfigParserTest, populateHandlerMap) {
  bool success = parser.Parse("test_configs/config_testing", &out_config);
  EXPECT_TRUE(success);
  std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap;
  out_config.populateHandlerMap(hdlrMap);
  EXPECT_EQ(hdlrMap.size(), 7);
  EXPECT_EQ(hdlrMap["/echo"].first, "EchoHandler");
  EXPECT_EQ(hdlrMap["/static1"].first, "StaticHandler");
  EXPECT_EQ(hdlrMap["/static1/text"].first, "StaticHandler");
  EXPECT_EQ(hdlrMap["/static1/images"].first, "StaticHandler");
  EXPECT_EQ(hdlrMap["/static2"].first, "StaticHandler");
  EXPECT_EQ(hdlrMap["/api"].first, "ApiHandler");
  EXPECT_EQ(hdlrMap["/echo"].second->ToString(), "");
  EXPECT_EQ(hdlrMap["/static1"].second->ToString(), "root ../data/data_etc;\n");
  EXPECT_EQ(hdlrMap["/static1/text"].second->ToString(), "root ../data/data_txt;\n");
  EXPECT_EQ(hdlrMap["/static1/images"].second->ToString(), "root ../data/data_img;\n");
  EXPECT_EQ(hdlrMap["/static2"].second->ToString(), "root ../data;\n");
  EXPECT_EQ(hdlrMap["/api"].second->ToString(), "root ../api_dir;\n");
  EXPECT_EQ(hdlrMap["/health"].second->ToString(), "");
}
