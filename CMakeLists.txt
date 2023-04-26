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
find_package(Boost 1.50 REQUIRED COMPONENTS system)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Locate bash program
find_program (BASH_PROGRAM bash)

# TODO(!): Update name and srcs
add_library(config_parser_lib src/config_parser.cc)
add_library(session_lib src/session.cc)
add_library(server_lib src/server.cc)

# TODO(!): Update executable name, srcs, and deps
add_executable(server src/server_main.cc)
target_link_libraries(server_lib session_lib)
target_link_libraries(server server_lib)
target_link_libraries(server Boost::system)
target_link_libraries(server pthread) # Needed for building with Windows dev environments
target_link_libraries(server config_parser_lib)

# TODO(!): Update test executable name, srcs, and deps
# add_executable(server_test.cc tests/example_lib_test.cc)
# target_link_libraries(example_lib_test example_lib gtest_main)
add_executable(config_parser_test tests/config_parser_test.cc)
add_executable(config_statement_test tests/config_statement_test.cc)
add_executable(config_test tests/config_test.cc)
add_executable(session_test tests/session_test.cc)
add_executable(server_test tests/server_test.cc)
target_link_libraries(config_parser_test config_parser_lib gtest_main)
target_link_libraries(config_statement_test config_parser_lib gtest_main)
target_link_libraries(config_test config_parser_lib gtest_main)
target_link_libraries(session_test session_lib gtest_main)
target_link_libraries(server_test server_lib gtest_main)

# TODO(!): Update with test binary
# gtest_discover_tests(example_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(config_statement_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(config_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(NAME integration_tests COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_tests.sh)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# TODO(!): Update with target/test targets
# generate_coverage_report(TARGETS example_server example_lib TESTS example_lib_test)

# TODO(!): Include integration tests here
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS session_lib server_lib config_parser_lib TESTS server_test session_test config_parser_test config_statement_test config_test)


