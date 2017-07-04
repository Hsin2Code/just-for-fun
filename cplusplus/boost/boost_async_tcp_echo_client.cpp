#include <iostream>
#include <cstdlib>
#include <deque>
#include <memory>
#include <boost/asio.hpp>

class handler
{
public:
    handler() {}
    ~handler() {}
    virtual void handle_connect(const boost::system::error_code &err) {};
private:
};
class session
{
public:
    // 构造函数
    session(handler &h) : m_socket(m_io_service)
                        , m_handler(h){
        m_is_connect = false;
    }
    // 析构函数
    ~session() { }
    void connect(std::string host, uint16_t port, uint32_t delayed_by_ms);
    void handle_connect(const boost::system::error_code &err);
private:
    boost::asio::io_service m_io_service;
    boost::asio::ip::tcp::socket m_socket;
    bool m_is_connect;

    handler m_handler;
};

using namespace boost::asio;
using boost::asio::ip::tcp;
using namespace std;

void
session::handle_connect( const boost::system::error_code &err )
{
    m_handler.handle_connect(err);
}
void
session::connect(string host, uint16_t port, uint32_t delayed_by_ms)
{
    m_socket.async_connect(tcp::endpoint(ip::address_v4::from_string(host), port),
                           bind(&session::handle_connect, this, std::placeholders::_1));
}


int main()
{
    return 0;
}
