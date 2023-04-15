#ifndef GOOFYGOOGLERSSERVER_SERVER_H_
#define GOOFYGOOGLERSSERVER_SERVER_H_

#include <boost/asio.hpp>

#include "session.h"

using boost::asio::ip::tcp;

class server {
public:
  server(boost::asio::io_service& io_service, short port);
private:
  void start_accept();
  void handle_accept(session* new_session,
      const boost::system::error_code& error);

  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
};

#endif  // GOOFYGOOGLERSSERVER_SERVER_H_