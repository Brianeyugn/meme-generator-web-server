cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)
project(goofy-googlers-server)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log_setup log)
message(STATUS "Boost version: ${Boost_VERSION}")
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

# Find SQLite3 library
set(sqlite_USE_STATIC_LIBS ON)
find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})
message(STATUS "SQLite3 version: ${SQLite3_VERSION}")

include_directories(include)

# Locate bash program
find_program (BASH_PROGRAM bash)

# Name and srcs
add_library(server_lib src/server.cc)
add_library(session_lib src/session.cc)
add_library(config_parser_lib src/config_parser.cc)
add_library(logging_lib src/logging.cc)
add_library(handler_lib src/request_handler.cc)
add_library(echo_request_lib src/echo_request_handler.cc)
add_library(static_request_lib src/static_request_handler.cc)
add_library(error_request_lib src/error_request_handler.cc)
add_library(api_request_lib src/api_request_handler.cc)
add_library(health_request_lib src/health_request_handler.cc)
add_library(meme_request_lib src/meme_request_handler.cc)
add_library(mime_lib src/mime.cc)
add_library(echo_handler_factory_lib src/echo_handler_factory.cc)
add_library(static_handler_factory_lib src/static_handler_factory.cc)
add_library(error_handler_factory_lib src/error_handler_factory.cc)
add_library(api_handler_factory_lib src/api_handler_factory.cc)
add_library(health_handler_factory_lib src/health_handler_factory.cc)
add_library(meme_handler_factory_lib src/meme_handler_factory.cc)
add_library(http_parser_lib src/http_parser.cc)

# Executable name, srcs, and deps
add_executable(server src/server_main.cc)
target_link_libraries(handler_lib echo_request_lib static_request_lib error_request_lib api_request_lib health_request_lib meme_request_lib)
target_link_libraries(static_request_lib handler_lib Boost::filesystem mime_lib)
target_link_libraries(echo_request_lib handler_lib)
target_link_libraries(error_request_lib handler_lib)
target_link_libraries(api_request_lib handler_lib Boost::filesystem mime_lib)
target_link_libraries(health_request_lib handler_lib mime_lib)
target_link_libraries(meme_request_lib handler_lib Boost::filesystem mime_lib libsqlite3.a)
target_link_libraries(session_lib http_parser_lib)
target_link_libraries(server server_lib session_lib Boost::system pthread config_parser_lib echo_request_lib static_request_lib error_request_lib api_request_lib health_request_lib meme_request_lib echo_handler_factory_lib static_handler_factory_lib error_handler_factory_lib api_handler_factory_lib health_handler_factory_lib meme_handler_factory_lib logging_lib Boost::regex Boost::log_setup Boost::log libsqlite3.a)
target_link_libraries(mime_lib)

# Test executable name, srcs, and deps
add_executable(api_handler_test tests/api_request_handler_test.cc)
add_executable(config_parser_test tests/config_parser_test.cc)
add_executable(echo_handler_test tests/echo_request_handler_test.cc)
add_executable(error_handler_test tests/error_request_handler_test.cc)
add_executable(health_handler_test tests/health_request_handler_test.cc)
add_executable(meme_handler_test tests/meme_request_handler_test.cc)
add_executable(http_parser_test tests/http_parser_test.cc)
add_executable(logging_test tests/logging_test.cc)
add_executable(mime_test tests/mime_test.cc)
add_executable(request_handler_test tests/request_handler_test.cc)
add_executable(server_test tests/server_test.cc)
add_executable(session_test tests/session_test.cc)
add_executable(static_handler_test tests/static_request_handler_test.cc)
target_link_libraries(api_handler_test api_request_lib config_parser_lib gtest_main logging_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(config_parser_test config_parser_lib gtest_main logging_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(echo_handler_test echo_request_lib gtest_main logging_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(error_handler_test error_request_lib gtest_main logging_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(health_handler_test health_request_lib config_parser_lib gtest_main logging_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(meme_handler_test meme_request_lib config_parser_lib gtest_main logging_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(http_parser_test http_parser_lib gtest_main logging_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(logging_test gtest_main logging_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(mime_test mime_lib gtest_main)
target_link_libraries(request_handler_test handler_lib gtest_main)
target_link_libraries(server_test server_lib session_lib echo_request_lib static_request_lib error_request_lib api_request_lib health_request_lib meme_request_lib echo_handler_factory_lib static_handler_factory_lib error_handler_factory_lib api_handler_factory_lib health_handler_factory_lib meme_handler_factory_lib config_parser_lib gtest_main logging_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(session_test session_lib echo_request_lib static_request_lib error_request_lib config_parser_lib gtest_main gmock_main logging_lib Boost::regex Boost::log_setup Boost::log)
target_link_libraries(static_handler_test static_request_lib config_parser_lib gtest_main logging_lib Boost::regex Boost::log_setup Boost::log mime_lib)

# Test binaries
gtest_discover_tests(api_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(echo_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(error_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(health_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(meme_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(http_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(logging_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(mime_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(static_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_test(NAME integration_tests COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/integration_tests.sh)

include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(
    TARGETS
        api_request_lib config_parser_lib echo_request_lib error_request_lib handler_lib health_request_lib meme_request_lib http_parser_lib logging_lib mime_lib handler_lib server_lib server session_lib static_request_lib
        logging_test request_handler_test # these are so we can cover the inlined header functions
    TESTS
        api_handler_test config_parser_test echo_handler_test error_handler_test health_handler_test meme_handler_test http_parser_test logging_test mime_test request_handler_test server_test session_test static_handler_test
)
