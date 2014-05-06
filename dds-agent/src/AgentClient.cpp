// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentClient.h"
// STD
#include <iostream>
#include <functional>

using namespace std::placeholders;
using namespace boost::asio;

CAgentClient::CAgentClient()
{
}

CAgentClient::~CAgentClient()
{
}

void CAgentClient::start()
{
    ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);
    for (int i = 0; i < 100; i++)
    {
        sleep(1);
        echo(ep, "Message 1");
    }
}

void CAgentClient::stop()
{
}

size_t read_complete(char* buf, const boost::system::error_code& err, size_t bytes)
{
    if (err)
        return 0;
    bool found = std::find(buf, buf + bytes, '\n') < buf + bytes;
    return found ? 0 : 1;
}

void CAgentClient::echo(const ip::tcp::endpoint& _ep, const std::string& _message)
{
    std::string message = _message + "\n";
    ip::tcp::socket sock(m_service);
    sock.connect(_ep);
    sock.write_some(buffer(message));
    char buf[1024];
    int bytes = read(sock, buffer(buf), std::bind(read_complete, buf, _1, _2));
    // int bytes = sock.read_some(buffer(buf));
    std::string copy(buf, bytes - 1);
    message = message.substr(0, message.size() - 1);
    std::cout << "Server echo: " << message << ": " << (copy == message ? "OK" : "FAIL") << std::endl;
    sock.close();
}
