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