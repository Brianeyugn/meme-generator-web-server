#ifndef GOOFYGOOGLERSSERVER_SERVER_H_
#define GOOFYGOOGLERSSERVER_SERVER_H_

#include <boost/asio.hpp>

#include "config_parser.h"
#include "session.h"

using boost::asio::ip::tcp;

class Server {
 public:
  Server(boost::asio::io_service& io_service, short port, const NginxConfig& config);
  bool IsRunning() const;
  int GetActiveSessions() const;

  void Stop();
  void Start();

  void HandleAccept(Session* new_session,
      const boost::system::error_code& error);

 private:
  void StartAccept_();
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  bool is_running_;
  int active_sessions_;
  NginxConfig config_;
};

#endif  // GOOFYGOOGLERSSERVER_SERVER_H_
