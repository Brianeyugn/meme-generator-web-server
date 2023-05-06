#include "logging.h"

#include <csignal>

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
  log = Logger::GetLogger();
  bool result = log == nullptr;
  EXPECT_FALSE(result);
}

TEST_F(LoggingTest, InstantiateSecondLogger) {
  EXPECT_DEATH(
    {
      log = Logger::GetLogger();
      Logger *log_1 = new Logger();
      Logger *log_2 = new Logger();
    },
    "Cannot instantiate new Logger object as one already exists"
  );
}

TEST_F(LoggingTest, TestSigHandler) {
  EXPECT_EXIT(
    {
      log = Logger::GetLogger();
      std::signal(SIGINT, log->SignalHandler);
      std::raise(SIGINT);
    },
    testing::ExitedWithCode(SIGINT),
    ".*"
  );
}

TEST_F(LoggingTest, LogTrace) {
  log = Logger::GetLogger();
  log->LogTrace("test message");
  SUCCEED();
}

TEST_F(LoggingTest, LogDebug) {
  log = Logger::GetLogger();
  log->LogDebug("test message");
  SUCCEED();
}

TEST_F(LoggingTest, LogInfo) {
  log = Logger::GetLogger();
  log->LogInfo("test message");
  SUCCEED();
}

TEST_F(LoggingTest, LogWarning) {
  log = Logger::GetLogger();
  log->LogWarn("test message");
  SUCCEED();
}

TEST_F(LoggingTest, LogError) {
  log = Logger::GetLogger();
  log->LogError("test message");
  SUCCEED();
}

TEST_F(LoggingTest, LogFatal) {
  log = Logger::GetLogger();
  log->LogFatal("test message");
  SUCCEED();
}

TEST_F(LoggingTest, DeleteAndRemakeLogger) {
  log = Logger::GetLogger();
  delete log;
  Logger *new_log = new Logger;

  bool result = new_log == nullptr;
  EXPECT_FALSE(result);
}
