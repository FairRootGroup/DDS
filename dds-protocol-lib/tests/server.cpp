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

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: dds-protocol-lib-server-test <port> [<port> ...]\n";
            return 1;
        }

        boost::asio::io_service io_service;

        /*        std::list<chat_server> servers;
                for (int i = 1; i < argc; ++i)
                {
                    tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
                    servers.emplace_back(io_service, endpoint);
                }
        */
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}