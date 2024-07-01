#include "websocket_client.hpp"

WebSocketClient::WebSocketClient() : m_open(false)
{
    m_client.init_asio();
    m_client.set_access_channels(websocketpp::log::alevel::all);
    m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
    m_client.set_open_handshake_timeout(5000);
    m_client.set_open_handler(std::bind(&WebSocketClient::on_open, this, std::placeholders::_1));
    m_client.set_close_handler(std::bind(&WebSocketClient::on_close, this, std::placeholders::_1));
}

WebSocketClient::~WebSocketClient()
{
    close();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void WebSocketClient::connect(const std::string &uri)
{
    websocketpp::lib::error_code ec;
    client::connection_ptr con = m_client.get_connection(uri, ec);

    if (ec)
    {
        std::cout << "Could not create connection because: " << ec.message() << std::endl;
        return;
    }

    m_hdl = con->get_handle();
    m_client.connect(con);

    m_thread = std::thread([this]()
                           { m_client.run(); });
}

void WebSocketClient::close()
{
    if (m_open)
    {
        websocketpp::lib::error_code ec;
        m_client.close(m_hdl, websocketpp::close::status::going_away, "", ec);
        if (ec)
        {
            std::cout << "Error initiating close: " << ec.message() << std::endl;
        }
    }
}

void WebSocketClient::send(const std::string &message)
{
    if (m_open)
    {
        websocketpp::lib::error_code ec;
        m_client.send(m_hdl, message, websocketpp::frame::opcode::text, ec);
        if (ec)
        {
            std::cout << "Send Error: " << ec.message() << std::endl;
        }
    }
}

void WebSocketClient::on_open(websocketpp::connection_hdl hdl)
{
    m_open = true;
    std::cout << "Connection opened" << std::endl;
}

void WebSocketClient::on_close(websocketpp::connection_hdl hdl)
{
    m_open = false;
    std::cout << "Connection closed" << std::endl;
}
