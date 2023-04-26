#ifndef GOOFYGOOGLERSSERVER_SERVER_H_
#define GOOFYGOOGLERSSERVER_SERVER_H_

#include <boost/asio.hpp>

#include "session.h"

using boost::asio::ip::tcp;

class server {
public:
  server(boost::asio::io_service& io_service, short port);
  bool is_running() const;
  int get_active_sessions() const;
  void stop();
  void start();
  void handle_accept(session* new_session,
      const boost::system::error_code& error);
private:
  void start_accept();
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  bool is_running_;
  int active_sessions_;
};

#endif  // GOOFYGOOGLERSSERVER_SERVER_H_