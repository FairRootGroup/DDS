// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// STD
#include <iostream>
// BOOST
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;

//----------------------------------------------------------------------

// class session_participant
//{
//  public:
//    virtual ~session_participant()
//    {
//    }
//    virtual void deliver(const chat_message& msg) = 0;
//};
//
// typedef std::shared_ptr<chat_participant> chat_participant_ptr;
//
////----------------------------------------------------------------------
//
// class server_session : public chat_participant, public std::enable_shared_from_this<chat_session>
//{
//  public:
//    chat_session(tcp::socket socket, chat_room& room)
//        : socket_(std::move(socket))
//        , room_(room)
//    {
//    }
//
//    void start()
//    {
//        room_.join(shared_from_this());
//        do_read_header();
//    }
//
//    void deliver(const chat_message& msg)
//    {
//        bool write_in_progress = !write_msgs_.empty();
//        write_msgs_.push_back(msg);
//        if (!write_in_progress)
//        {
//            do_write();
//        }
//    }
//
//  private:
//    void do_read_header()
//    {
//        auto self(shared_from_this());
//        boost::asio::async_read(socket_,
//                                boost::asio::buffer(read_msg_.data(), chat_message::header_length),
//                                [this, self](boost::system::error_code ec, std::size_t /*length*/)
//                                {
//            if (!ec && read_msg_.decode_header())
//            {
//                do_read_body();
//            }
//            else
//            {
//                room_.leave(shared_from_this());
//            }
//        });
//    }
//
//    void do_read_body()
//    {
//        auto self(shared_from_this());
//        boost::asio::async_read(socket_,
//                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
//                                [this, self](boost::system::error_code ec, std::size_t /*length*/)
//                                {
//            if (!ec)
//            {
//                room_.deliver(read_msg_);
//                do_read_header();
//            }
//            else
//            {
//                room_.leave(shared_from_this());
//            }
//        });
//    }
//
//    void do_write()
//    {
//        auto self(shared_from_this());
//        boost::asio::async_write(socket_,
//                                 boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
//                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
//                                 {
//            if (!ec)
//            {
//                write_msgs_.pop_front();
//                if (!write_msgs_.empty())
//                {
//                    do_write();
//                }
//            }
//            else
//            {
//                room_.leave(shared_from_this());
//            }
//        });
//    }
//
//    tcp::socket socket_;
//    chat_room& room_;
//    chat_message read_msg_;
//    chat_message_queue write_msgs_;
//};

//----------------------------------------------------------------------

class test_server
{
  public:
    test_server(boost::asio::io_service& io_service, const tcp::endpoint& endpoint)
        : m_acceptor(io_service, endpoint)
        , m_socket(io_service)
    {
        do_accept();
    }

  private:
    void do_accept()
    {
        m_acceptor.async_accept(m_socket,
                                [this](boost::system::error_code ec)
                                {
            if (!ec)
            {
                //                std::make_shared<chat_session>(std::move(m_socket), m_room)->start();
            }

            do_accept();
        });
    }

    tcp::acceptor m_acceptor;
    tcp::socket m_socket;
    // chat_room m_room;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: dds-protocol-lib-server-test <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[1]));
        test_server srv(io_service, endpoint);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}