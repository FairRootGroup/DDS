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
        if (argc != 3)
        {
            std::cerr << "Usage: dds-protocol-lib-client-tests <host> <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        /*        tcp::resolver resolver(io_service);
                auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
                chat_client c(io_service, endpoint_iterator);

                std::thread t([&io_service]()
                              { io_service.run(); });

                char line[chat_message::max_body_length + 1];
                while (std::cin.getline(line, chat_message::max_body_length + 1))
                {
                    chat_message msg;
                    msg.body_length(std::strlen(line));
                    std::memcpy(msg.body(), line, msg.body_length());
                    msg.encode_header();
                    c.write(msg);
                }

                c.close();
                t.join();*/
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}