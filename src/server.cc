#include "server.h"

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "logging.h"
#include "session.h"

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_service& io_service, short port)
  : io_service_(io_service),
  is_running_(false),
  active_sessions_(0),
  acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
  Logger *log = Logger::GetLogger();
  log->LogInfo("Server running on port: " + std::to_string(port));

  StartAccept();
}

// Start accepting requests to the Server
void Server::StartAccept() {
  Session* new_session = new Session(io_service_);
  acceptor_.async_accept(new_session->Socket(),
      boost::bind(&Server::HandleAccept, this, new_session,
          boost::asio::placeholders::error));
}

// TODO: Unused
bool Server::IsRunning() const {
  return is_running_;
}

// TODO: Unused
void Server::Start() {
  if (!is_running_) {
    is_running_ = true;
    StartAccept();
  }
}

// TODO: Unused
void Server::Stop() {
  if (is_running_) {
    is_running_ = false;
    acceptor_.cancel();
  }
}

// TODO: Unused
int Server::GetActiveSessions() const {
  return active_sessions_;
}

// Start a new request Session
void Server::HandleAccept(Session* new_session,
  const boost::system::error_code& error) {
  if (!error) {
    new_session->Start();
    active_sessions_++;
  } else {
    delete new_session;
  }

  StartAccept();
  active_sessions_--;
}
