cmake_minimum_required(VERSION 3.10.0)
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

include_directories(include)

# Locate bash program
find_program (BASH_PROGRAM bash)

# Name and srcs
add_library(config_parser_lib src/config_parser.cc)
add_library(echo_request_handler_lib src/echo_request_handler.cc)
add_library(logging_lib src/logging.cc)
add_library(request_handler_lib src/request_handler.cc)
add_library(server_lib src/server.cc src/session.cc src/logging.cc)
add_library(session_lib src/session.cc src/request_handler.cc src/echo_request_handler.cc src/static_request_handler.cc src/logging.cc)
add_library(static_request_handler_lib src/static_request_handler.cc)

# Executable name, srcs, and deps
add_executable(server src/server_main.cc)
target_link_libraries(echo_request_handler_lib request_handler_lib)
target_link_libraries(logging_lib Boost::log_setup Boost::log)
target_link_libraries(static_request_handler_lib request_handler_lib)
target_link_libraries(session_lib echo_request_handler_lib)
target_link_libraries(session_lib logging_lib)
target_link_libraries(session_lib request_handler_lib)
target_link_libraries(session_lib static_request_handler_lib)
target_link_libraries(server_lib logging_lib)
target_link_libraries(server_lib session_lib)
target_link_libraries(server config_parser_lib)
target_link_libraries(server logging_lib)
target_link_libraries(server pthread) # Needed for building with Windows dev environments
target_link_libraries(server server_lib)

# Test executable name, srcs, and deps
add_executable(config_parser_test tests/config_parser_test.cc)
add_executable(config_statement_test tests/config_statement_test.cc)
add_executable(config_test tests/config_test.cc)
add_executable(session_test tests/session_test.cc)
add_executable(server_test tests/server_test.cc)
add_executable(request_handler_test tests/request_handler_test.cc)
add_executable(echo_request_handler_test tests/echo_request_handler_test.cc)
add_executable(static_request_handler_test tests/static_request_handler_test.cc)
add_executable(logging_test tests/logging_test.cc)
target_link_libraries(config_parser_test config_parser_lib gtest_main)
target_link_libraries(config_statement_test config_parser_lib gtest_main)
target_link_libraries(config_test config_parser_lib gtest_main)
target_link_libraries(session_test session_lib gtest_main)
target_link_libraries(server_test server_lib gtest_main)
target_link_libraries(request_handler_test request_handler_lib gtest_main)
target_link_libraries(echo_request_handler_test echo_request_handler_lib gtest_main)
target_link_libraries(static_request_handler_test static_request_handler_lib gtest_main)
target_link_libraries(logging_test logging_lib gtest_main)

# Test binaries
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(config_statement_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(config_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(echo_request_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(static_request_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(logging_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
#add_test(NAME integration_tests COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_tests.sh)

include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(
    TARGETS
        session_lib server_lib config_parser_lib echo_request_handler_lib static_request_handler_lib request_handler_lib logging_lib
    TESTS
        server_test session_test config_parser_test config_statement_test config_test request_handler_test echo_request_handler_test static_request_handler_test logging_test
)
