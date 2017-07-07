#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iostream>
#include <boost/asio.hpp>

class chat_message
{
public:
    enum { header_length = 4 };
    enum { max_body_length = 1024 };

    chat_message()
        : body_length_(0) {
        memset(data_, 0, max_body_length + header_length);
    }

    const char* data() const {
        return data_;
    }

    char* data() {
        return data_;
    }

    size_t length() const {
        return header_length + body_length_;
    }

    const char* body() const {
        return data_ + header_length;
    }

    char* body() {
        return data_ + header_length;
    }

    size_t body_length() const {
        return body_length_;
    }

    void body_length(size_t length) {
        body_length_ = length;
        if (body_length_ > max_body_length)
            body_length_ = max_body_length;
    }

    bool decode_header() {
        body_length_ = data_[3] & 0xFF;
        body_length_ |= ((data_[2] << 8) & 0xFF00);
        body_length_ |= ((data_[1] << 16) & 0xFF0000);
        body_length_ |= ((data_[0] << 24) & 0xFF000000);
        if (body_length_ > max_body_length) {
            body_length_ = 0;
            return false;
        }
        return true;
    }

    void encode_header() {
        memset(data_, 0 , 4);
        data_[3] = (char) (0xff & body_length_);
        data_[2] = (char) ((0xff00 & body_length_) >> 8);
        data_[1] = (char) ((0xff0000 & body_length_) >> 16);
        data_[0] = (char) ((0xff000000 & body_length_) >> 24);
    }

private:
    char data_[header_length + max_body_length];
    size_t body_length_;
};

using boost::asio::ip::tcp;

class chat_client
{
    typedef std::deque<chat_message> chat_message_queue;
private:
    boost::asio::io_service io_service_;
    std::shared_ptr<tcp::socket> socket_;
    boost::asio::deadline_timer timer_;
    boost::asio::deadline_timer heartbeat_timer_; // 业务逻辑:心跳计时器
    bool bConnected_;
    bool bWantReconnect_;
    pthread_t tid_;
public:
    chat_client() : socket_(new tcp::socket(io_service_)),
                    timer_(io_service_),
                    heartbeat_timer_(io_service_),
                    bConnected_(false),
                    bWantReconnect_(false),
                    tid_(0) {
    }
    ~chat_client() {
        disconnect();
        io_service_.stop();
    }
    void run() {
        io_service_.run();
    }
    void connect(std::string host, std::string port, bool bReconnect = true) {
        bWantReconnect_ = bReconnect;
        tcp::resolver resolver(io_service_);
        tcp::resolver::query query(host, port);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::endpoint endpoint = *endpoint_iterator;
        socket_->async_connect(endpoint, std::bind(&chat_client::handle_connect, this,
                                                   std::placeholders::_1, ++endpoint_iterator));
        timer_.expires_from_now(boost::posix_time::seconds(5));
        timer_.async_wait(std::bind(&chat_client::handle_timeout, this));
    }
    void disconnect() {
        bWantReconnect_ = false;
        io_service_.post(std::bind(&chat_client::do_close, this));
    }
    void write(const chat_message& msg) {
        io_service_.post(std::bind(&chat_client::do_write, this, msg));
    }
    void heartbeat() {
        heartbeat_timer_.expires_from_now(boost::posix_time::seconds(10));
        heartbeat_timer_.async_wait(std::bind(&chat_client::do_heartbeat, this));
    }
private:
    void handle_connect(const boost::system::error_code& error,
                        tcp::resolver::iterator endpoint_iterator) {
        if(!error) {
            bConnected_ = true;
            std::cout << "Connect - stating receive ..." << std::endl;
            boost::asio::async_read(*socket_, boost::asio::buffer(read_msg_.data(), chat_message::header_length),
                                    std::bind(&chat_client::handle_read_header, this, std::placeholders::_1));
            login();            // 业务逻辑
        }
        else if(endpoint_iterator != tcp::resolver::iterator()) {
            std::cout << "async connect" << std::endl;

            socket_->close();
            tcp::endpoint endpoint = *endpoint_iterator;
            socket_->async_connect(endpoint, std::bind(&chat_client::handle_connect, this,
                                                       std::placeholders::_1, ++endpoint_iterator));
        }
    }
    void handle_timeout() {
        if(bConnected_ == false) {
            do_close();
        }
    }
    void handle_read_header(const boost::system::error_code& error) {
        if(!error && read_msg_.decode_header()) {
            boost::asio::async_read(*socket_,
                                    boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                    bind(&chat_client::handle_read_body, this, std::placeholders::_1));
        }else {
            do_close();
        }
    }
    void handle_read_body(const boost::system::error_code& error) {
        if (!error) {
            std::string strJson(read_msg_.body(), read_msg_.body_length());
            io_service_.post(std::bind(&chat_client::handle_response, this, strJson));
            boost::asio::async_read(*socket_,
                                    boost::asio::buffer(read_msg_.data(), chat_message::header_length),
                                    std::bind(&chat_client::handle_read_header, this, std::placeholders::_1));
        }
        else {
            do_close();
        }
    }

    void do_write(chat_message msg) {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress && bConnected_) {
            std::cout << "SX: " << write_msgs_.front().body() << std::endl;
            boost::asio::async_write(*socket_,
                                     boost::asio::buffer(write_msgs_.front().data(),
                                                         write_msgs_.front().length()),
                                     std::bind(&chat_client::handle_write, this,
                                               std::placeholders::_1));
        }
    }

    void handle_write(const boost::system::error_code& error) {
        if (!error) {
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) {
                std::cout << "SX: " << write_msgs_.front().body() << std::endl;
                boost::asio::async_write(*socket_,
                                         boost::asio::buffer(write_msgs_.front().data(),
                                                             write_msgs_.front().length()),
                                         std::bind(&chat_client::handle_write, this,
                                                   std::placeholders::_1));
            }
        }
        else {
            do_close();
        }
    }

    void do_close() {
        io_service_.reset();
        if(socket_->is_open()) {
            socket_->close();  // close connection;
        }
        socket_.reset(new tcp::socket(io_service_));

        bConnected_ = false;
        if(bWantReconnect_) {
            reconnect();
        }else {
            io_service_.stop();
        }
    }
    virtual void do_heartbeat() {
        std::cout << "Do heartbeat ..." << std::endl;
    }
    virtual void login() {
        std::cout << "Login server..." << std::endl;
    }
    virtual void reconnect() {
        std::cout << "Re-Connecting ..." << std::endl;
    }
    virtual void handle_response(std::string strJson) {
        std::cout << "RX: " << strJson << std::endl;
    }
private:
    chat_message read_msg_;
    chat_message_queue write_msgs_;
};
