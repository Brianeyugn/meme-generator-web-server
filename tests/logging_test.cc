#include "logging.h"

#include "gtest/gtest.h"

class LoggingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    return;
  }

  Logger *log;
};

TEST_F(LoggingTest, LoggerInitiallyNull) {
  bool result = log == nullptr;
  EXPECT_TRUE(result);
}

TEST_F(LoggingTest, InstantiateLogger) {
  log = Logger::get_logger();
  bool result = log == nullptr;
  EXPECT_FALSE(result);
}

TEST_F(LoggingTest, InstantiateSecondLogger) {
  EXPECT_DEATH(
    {
      log = Logger::get_logger();
      Logger *log_1 = new Logger();
      Logger *log_2 = new Logger();
    },
    "Cannot instantiate new Logger object as one already exists"
  );
}

TEST_F(LoggingTest, LogTrace) {
  log = Logger::get_logger();
  log->log_trace("test message");
  SUCCEED();
}

TEST_F(LoggingTest, LogDebug) {
  log = Logger::get_logger();
  log->log_debug("test message");
  SUCCEED();
}

TEST_F(LoggingTest, LogInfo) {
  log = Logger::get_logger();
  log->log_info("test message");
  SUCCEED();
}

TEST_F(LoggingTest, LogWarning) {
  log = Logger::get_logger();
  log->log_warn("test message");
  SUCCEED();
}

TEST_F(LoggingTest, LogError) {
  log = Logger::get_logger();
  log->log_error("test message");
  SUCCEED();
}

TEST_F(LoggingTest, LogFatal) {
  log = Logger::get_logger();
  log->log_fatal("test message");
  SUCCEED();
}

TEST_F(LoggingTest, DeleteAndRemakeLogger) {
  log = Logger::get_logger();
  delete log;
  Logger *new_log = new Logger;

  bool result = new_log == nullptr;
  EXPECT_FALSE(result);
}
