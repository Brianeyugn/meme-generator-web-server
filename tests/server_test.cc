#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "config_parser.h"
#include "server.h"

class ServerTest : public ::testing::Test {
protected:
    boost::asio::io_service io_service;
    std::unique_ptr<Server> server_instance;

    ServerTest() : server_instance(nullptr) {}

    void SetUp() override {
      server_instance = std::make_unique<Server>(io_service, 1234);
    }
    
    void TearDown() override {
        server_instance->Stop();
        server_instance.reset();
    }
};

//Server starts and stops
TEST_F(ServerTest, ServerStartsAndStops) {
    server_instance->Start();
    EXPECT_TRUE(server_instance->IsRunning());
    server_instance->Stop();
    EXPECT_FALSE(server_instance->IsRunning());
}

TEST_F(ServerTest, ClientConnectsAndDisconnects) {
    // Check that the Server can accept a client connection and disconnect
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket client_socket(io_service);
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::connect(client_socket, resolver.resolve({"127.0.0.1", "1234"}));

    // The Session should Start automatically after the client connects
    boost::asio::deadline_timer timer(io_service, boost::posix_time::milliseconds(500));
    timer.wait();

    // The Session should have stopped after the client disconnects
    client_socket.close();
    timer.expires_from_now(boost::posix_time::milliseconds(500));
    timer.wait();
    EXPECT_EQ(server_instance->GetActiveSessions(), 0);
}

TEST_F(ServerTest, StartStopMultipleTimes) {
    // Check that the Server can be started and stopped multiple times
    for (int i = 0; i < 3; i++) {
        server_instance->Start();
        EXPECT_TRUE(server_instance->IsRunning());
        server_instance->Stop();
        EXPECT_FALSE(server_instance->IsRunning());
    }
}

TEST_F(ServerTest, ServerNotRunningOnConstruction) {
    // Check that the Server is not running on construction
    server_instance.reset();
    server_instance = std::make_unique<Server>(io_service, 1234);
    EXPECT_FALSE(server_instance->IsRunning());
}

/*
TEST_F(ServerTest, ServerStopsAcceptingOnStop) {
    // Check that the Server stops accepting connections after Stop is called
    server_instance->Start();
    EXPECT_TRUE(server_instance->IsRunning());
    server_instance->Stop();
    EXPECT_FALSE(server_instance->IsRunning());
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket client_socket(io_service);
    boost::asio::ip::tcp::resolver resolver(io_service);
    EXPECT_THROW(boost::asio::connect(client_socket, resolver.resolve({"127.0.0.1", "1234"})),
                 boost::system::system_error);
}

TEST_F(ServerTest, HandleAcceptWithError) {
    // Check that the Server deletes the Session object on error
    Session* new_session = new Session(io_service);
    new_session->socket().close();
    boost::system::error_code error = boost::asio::error::eof;
    server_instance->HandleAccept(new_session, error);
    EXPECT_EQ(server_instance->GetActiveSessions(), 0);
    delete new_session;
}
*/