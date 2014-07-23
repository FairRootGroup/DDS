// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentClient.h"
#include "Logger.h"
#include "ProtocolCommands.h"
// STD
#include <functional>
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace std::placeholders;
using namespace boost::asio;
using namespace MiscCommon;
using namespace dds;

CAgentClient::CAgentClient(boost::asio::io_service& _service)
    : m_socket(_service)
    , m_service(_service)
{
}

CAgentClient::~CAgentClient()
{
}

void CAgentClient::start()
{
    LOG(info) << "Starting agent...";

    // Read server info file
    const string sSrvCfg(CUserDefaults::getServerInfoFile());
    LOG(info) << "Reading server info from: " << sSrvCfg;
    if (sSrvCfg.empty())
        throw runtime_error("Can't find server info file.");

    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(sSrvCfg, pt);
    const string sHost(pt.get<string>("server.host"));
    const int nPort(pt.get<int>("server.port"));

    LOG(info) << "Contacting DDS commander on " << sHost << ":" << nPort;

    ip::tcp::endpoint ep(ip::address::from_string(sHost), nPort);
    m_socket.async_connect(ep, std::bind(&CAgentClient::connectHandler, this, std::placeholders::_1));
    m_service.run();
}

void CAgentClient::stop()
{
    LOG(info) << "Stoping agent...";
    m_service.stop();
    m_socket.close();
}

// void CAgentClient::readHandler(const boost::system::error_code& _ec, std::size_t _bytesTransferred)
//{
//    if (!_ec)
//    {
//        std::string msg(m_readBuffer, _bytesTransferred);
//        LOG(info) << "Server response: " << msg;
//        sleep(1);
//
//        doWrite("ping\n");
//    }
//}

// void CAgentClient::writeHandler(const boost::system::error_code& _ec, std::size_t _bytesTransferred)
//{
//    readHeader();
//}

void CAgentClient::connectHandler(const boost::system::error_code& _ec)
{
    if (!_ec)
    {
        writeMessage();
        // readHeader();
        // doWrite("ping\n");
        LOG(debug) << "Connection established";
    }
    else
    {
        LOG(fatal) << "Unable to connect.";
    }
}

// size_t CAgentClient::readCompleteHandler(const boost::system::error_code& _ec, size_t _bytesTransferred)
//{
//    if (_ec)
//        return 0;
//    bool found = std::find(m_readBuffer, m_readBuffer + _bytesTransferred, '\n') < m_readBuffer + _bytesTransferred;
//    return found ? 0 : 1;
//}
//
// void CAgentClient::doRead()
//{
//    async_read(m_socket,
//               boost::asio::buffer(m_readBuffer),
//               std::bind(&CAgentClient::readCompleteHandler, this, std::placeholders::_1, std::placeholders::_2),
//               std::bind(&CAgentClient::readHandler, this, std::placeholders::_1, std::placeholders::_2));
//}
//
// void CAgentClient::doWrite(const std::string& msg)
//{
//    std::copy(msg.begin(), msg.end(), m_writeBuffer);
//    async_write(
//        m_socket, boost::asio::buffer(m_writeBuffer, msg.size()), std::bind(&CAgentClient::writeHandler, this, std::placeholders::_1, std::placeholders::_2));
//}

void CAgentClient::readHeader()
{
    auto readHandler = [this](boost::system::error_code ec, std::size_t /*length*/)
    {
        if (!ec && m_currentMsg.decode_header())
        {
            // If the header is ok, recieve the body of the message
            readBody();
        }
        else
        {
            stop();
        }
    };

    boost::asio::async_read(m_socket, boost::asio::buffer(m_currentMsg.data(), CProtocolMessage::header_length), readHandler);
}

void CAgentClient::readBody()
{
    //    auto self(shared_from_this());
    boost::asio::async_read(m_socket,
                            boost::asio::buffer(m_currentMsg.body(), m_currentMsg.body_length()),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
        if (!ec)
        {
            LOG(debug) << "Received from Agent: " << m_currentMsg.toString();
            // process recieved message
            processMessage();
            // Read next message
            readHeader();
        }
        else
        {
            stop();
        }
    });
}

void CAgentClient::processMessage()
{
    switch (m_currentMsg.header().m_cmd)
    {
        case cmdHANDSHAKE:
            SVersionCmd ver;
            ver.convertFromData(m_currentMsg.bodyToContainer());
            // send shutdown if versions are incompatible
            if (ver != SVersionCmd())
            {
                LOG(warning) << "Client's protocol version is incompatable. Client: " << m_socket.remote_endpoint().address().to_string();
                CProtocolMessage msg;
                msg.encode_message(cmdSHUTDOWN, BYTEVector_t());

                // auto self(shared_from_this());
                async_write(m_socket,
                            boost::asio::buffer(msg.data(), msg.length()),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                    if (!ec)
                    {
                        // write_msgs_.pop_front();
                        // if (!write_msgs_.empty())
                        // {
                        //     do_write();
                        // }
                    }
                    else
                    {
                        // room_.leave(shared_from_this());
                    }
                });
            }
            break;
    }
}

void CAgentClient::writeMessage()
{
    auto writeHandler = [this](boost::system::error_code ec, std::size_t /*_bytesTransferred*/)
    {
        if (!ec)
        {
            // If the header is ok, recieve the body of the message
            // readBody();
            LOG(info) << "Data successfully sent";
        }
        else
        {
            // stop();
            LOG(info) << "Error sending data";
        }
    };

    SVersionCmd ver_src;
    // ver_src.m_version = 2;
    BYTEVector_t data_to_send;
    ver_src.convertToData(&data_to_send);
    CProtocolMessage msg;
    msg.encode_message(cmdHANDSHAKE, data_to_send);

    async_write(m_socket, boost::asio::buffer(msg.data(), msg.length()), writeHandler);
}
