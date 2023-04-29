#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "server.h"
#include "session.h"
#include "logging.h"

using boost::asio::ip::tcp;

server::server(boost::asio::io_service& io_service, short port)
  : io_service_(io_service),
  is_running_(false),
  active_sessions_(0),
  acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
  BOOST_LOG_TRIVIAL(info) << "Server Running on Port: " << port;
  start_accept();
}


void server::start_accept() {
  session* new_session = new session(io_service_);
  acceptor_.async_accept(new_session->socket(),
    boost::bind(&server::handle_accept, this, new_session,
      boost::asio::placeholders::error));
}

bool server::is_running() const {
  return is_running_;
}

void server::start() {
  if (!is_running_) {
    is_running_ = true;
    start_accept();
  }
}

void server::stop() {
  if (is_running_) {
    is_running_ = false;
    acceptor_.cancel();
  }
}

int server::get_active_sessions() const {
  return active_sessions_;
}

void server::handle_accept(session* new_session,
  const boost::system::error_code& error) {
  if (!error) {
    new_session->start();
    active_sessions_++;
  } else {
    delete new_session;
  }

  start_accept();
  active_sessions_--;
}
