#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <string>
#include <thread>

class WebSocketClient
{
public:
  WebSocketClient();
  ~WebSocketClient();

  void connect(const std::string &uri);
  void close();
  void send(const std::string &message);

private:
  typedef websocketpp::client<websocketpp::config::asio_client> client;

  client m_client;
  websocketpp::connection_hdl m_hdl;
  std::thread m_thread;
  bool m_open;

  void on_open(websocketpp::connection_hdl hdl);
  void on_close(websocketpp::connection_hdl hdl);
  void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);
};

#endif