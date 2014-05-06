// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "CommanderServer.h"

using namespace boost::asio;
using namespace std::placeholders;
using namespace std;

CCommanderServer::CCommanderServer()
{
}

CCommanderServer::~CCommanderServer()
{
}

size_t read_complete(char* buff, const boost::system::error_code& err, size_t bytes)
{
    if (err)
        return 0;
    bool found = std::find(buff, buff + bytes, '\n') < buff + bytes;
    return found ? 0 : 1;
}

void CCommanderServer::start()
{
    ip::tcp::acceptor acceptor(m_service, ip::tcp::endpoint(ip::tcp::v4(), 8001));
    char buff[1024];
    while (true)
    {
        ip::tcp::socket sock(m_service);
        acceptor.accept(sock);
        int bytes = read(sock, buffer(buff), bind(read_complete, buff, _1, _2));
        // int bytes = sock.read_some(buffer(buff));
        std::string msg(buff, bytes);
        sock.write_some(buffer(msg));
        sock.close();
    }
}

void CCommanderServer::stop()
{
}
