#include "logging.h"

#include <csignal>

#include "gtest/gtest.h"

class LoggingTestFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    return;
  }

  Logger *log;
};

TEST_F(LoggingTestFixture, InstantiateLogger) {
  log = Logger::GetLogger();
  bool result = log == nullptr;
  EXPECT_FALSE(result);
}

TEST_F(LoggingTestFixture, InstantiateSecondLogger) {
  EXPECT_DEATH(
    {
      log = Logger::GetLogger();
      Logger *log_1 = new Logger();
      Logger *log_2 = new Logger();
    },
    "Cannot instantiate new Logger object as one already exists"
  );
}

TEST_F(LoggingTestFixture, TestSigHandler) {
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

TEST_F(LoggingTestFixture, LogTrace) {
  log = Logger::GetLogger();
  log->LogTrace("test message");
  SUCCEED();
}

TEST_F(LoggingTestFixture, LogDebug) {
  log = Logger::GetLogger();
  log->LogDebug("test message");
  SUCCEED();
}

TEST_F(LoggingTestFixture, LogInfo) {
  log = Logger::GetLogger();
  log->LogInfo("test message");
  SUCCEED();
}

TEST_F(LoggingTestFixture, LogWarning) {
  log = Logger::GetLogger();
  log->LogWarn("test message");
  SUCCEED();
}

TEST_F(LoggingTestFixture, LogError) {
  log = Logger::GetLogger();
  log->LogError("test message");
  SUCCEED();
}

TEST_F(LoggingTestFixture, LogFatal) {
  log = Logger::GetLogger();
  log->LogFatal("test message");
  SUCCEED();
}

TEST_F(LoggingTestFixture, DeleteAndRemakeLogger) {
  log = Logger::GetLogger();
  delete log;
  Logger *new_log = new Logger;

  bool result = new_log == nullptr;
  EXPECT_FALSE(result);
}
