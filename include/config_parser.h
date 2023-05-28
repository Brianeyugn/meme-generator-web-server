#ifndef GOOFYGOOGLERSSERVER_CONFIG_PARSER_H_
#define GOOFYGOOGLERSSERVER_CONFIG_PARSER_H_

// An nginx config file parser.

#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <utility>
#include <vector>

class NginxConfig;

// The parsed representation of a single config statement.
class NginxConfigStatement {
 public:
  std::string ToString(int depth);
  std::vector<std::string> tokens_;
  std::unique_ptr<NginxConfig> child_block_;
};

// The parsed representation of the entire config.
class NginxConfig {
 public:
  std::string ToString(int depth = 0);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements_;
  int GetPort();
  int GetPortHelper();
  std::map<std::string, std::string> GetFilesystemPath();
  void GetFilesystemPathHelper(std::map<std::string, std::string>& fsPaths);
  std::string removeSlash(std::string path);

  void populateHandlerMap(std::map<std::string, std::pair<std::string, NginxConfig*>>& handler_map);
};

// The driver that parses a config file and generates an NginxConfig.
class NginxConfigParser {
 public:
  NginxConfigParser() {}

  bool Parse(std::istream* config_file, NginxConfig* config);
  bool Parse(const char* file_name, NginxConfig* config);

 private:
  enum TokenType {
    TOKEN_TYPE_START = 0,
    TOKEN_TYPE_NORMAL = 1,
    TOKEN_TYPE_START_BLOCK = 2,
    TOKEN_TYPE_END_BLOCK = 3,
    TOKEN_TYPE_COMMENT = 4,
    TOKEN_TYPE_STATEMENT_END = 5,
    TOKEN_TYPE_EOF = 6,
    TOKEN_TYPE_ERROR = 7,
    TOKEN_TYPE_QUOTED_STRING = 8,
    TOKEN_TYPE_WHITESPACE = 9
  };
  const char* TokenTypeAsString(TokenType type);

  enum TokenParserState {
    TOKEN_STATE_INITIAL_WHITESPACE = 0,
    TOKEN_STATE_SINGLE_QUOTE = 1,
    TOKEN_STATE_DOUBLE_QUOTE = 2,
    TOKEN_STATE_TOKEN_TYPE_COMMENT = 3,
    TOKEN_STATE_TOKEN_TYPE_NORMAL = 4,
    TOKEN_STATE_WHITESPACE = 5
  };

  TokenType ParseToken(std::istream* input, std::string* value);
};

#endif  // GOOFYGOOGLERSSERVER_CONFIG_PARSER_H_
