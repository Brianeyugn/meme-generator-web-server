#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "config_parser.h"
#include "server.h"

class ServerTest : public ::testing::Test {
protected:
    boost::asio::io_service io_service;
    std::unique_ptr<server> server_instance;

    ServerTest() : server_instance(nullptr) {}

    void SetUp() override {
      server_instance = std::make_unique<server>(io_service, 1234);
    }
    
    void TearDown() override {
        server_instance->stop();
        server_instance.reset();
    }
};

//Server starts and stops
TEST_F(ServerTest, ServerStartsAndStops) {
    server_instance->start();
    EXPECT_TRUE(server_instance->is_running());
    server_instance->stop();
    EXPECT_FALSE(server_instance->is_running());
}

TEST_F(ServerTest, ClientConnectsAndDisconnects) {
    // Check that the server can accept a client connection and disconnect
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket client_socket(io_service);
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::connect(client_socket, resolver.resolve({"127.0.0.1", "1234"}));

    // The session should start automatically after the client connects
    boost::asio::deadline_timer timer(io_service, boost::posix_time::milliseconds(500));
    timer.wait();

    // The session should have stopped after the client disconnects
    client_socket.close();
    timer.expires_from_now(boost::posix_time::milliseconds(500));
    timer.wait();
    EXPECT_EQ(server_instance->get_active_sessions(), 0);
}

TEST_F(ServerTest, StartStopMultipleTimes) {
    // Check that the server can be started and stopped multiple times
    for (int i = 0; i < 3; i++) {
        server_instance->start();
        EXPECT_TRUE(server_instance->is_running());
        server_instance->stop();
        EXPECT_FALSE(server_instance->is_running());
    }
}

TEST_F(ServerTest, ServerNotRunningOnConstruction) {
    // Check that the server is not running on construction
    server_instance.reset();
    server_instance = std::make_unique<server>(io_service, 1234);
    EXPECT_FALSE(server_instance->is_running());
}

/*
TEST_F(ServerTest, ServerStopsAcceptingOnStop) {
    // Check that the server stops accepting connections after stop is called
    server_instance->start();
    EXPECT_TRUE(server_instance->is_running());
    server_instance->stop();
    EXPECT_FALSE(server_instance->is_running());
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket client_socket(io_service);
    boost::asio::ip::tcp::resolver resolver(io_service);
    EXPECT_THROW(boost::asio::connect(client_socket, resolver.resolve({"127.0.0.1", "1234"})),
                 boost::system::system_error);
}

TEST_F(ServerTest, HandleAcceptWithError) {
    // Check that the server deletes the session object on error
    session* new_session = new session(io_service);
    new_session->socket().close();
    boost::system::error_code error = boost::asio::error::eof;
    server_instance->handle_accept(new_session, error);
    EXPECT_EQ(server_instance->get_active_sessions(), 0);
    delete new_session;
}
*/