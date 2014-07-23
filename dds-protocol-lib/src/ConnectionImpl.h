// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__Connection__
#define __DDS__Connection__
// STD
#include <iostream>
#include <deque>
// BOOST
#include "boost/noncopyable.hpp"
#include "boost/asio.hpp"
// DDS
#include "ProtocolMessage.h"
#include "ProtocolCommands.h"
#include "Logger.h"

#define BEGIN_MSG_MAP(theClass)                                          \
  public:                                                                \
    void processMessage(const CProtocolMessage& _currentMsg, int& _nRes) \
    {                                                                    \
        switch (_currentMsg.header().m_cmd)                              \
        {

#define MESSAGE_HANDLER(msg, func) \
    case msg:                      \
        _nRes = func(_currentMsg); \
        break;

#define END_MSG_MAP()                                                                                        \
    default:                                                                                                 \
        LOG(MiscCommon::error) << "The received message doesn't have a handler: " << _currentMsg.toString(); \
        }                                                                                                    \
        }

namespace dds
{
    template <class T>
    class CConnectionImpl : public boost::noncopyable
    {
      protected:
        CConnectionImpl<T>(boost::asio::io_service& _service)
            : m_socket(_service)
            , m_started(false)
            , m_currentMsg()
            , m_outputMessageQueue()
        {
        }

      public:
        typedef std::shared_ptr<T> connectionPtr_t;
        typedef std::vector<connectionPtr_t> connectionPtrVector_t;
        typedef std::deque<CProtocolMessage> messageQueue_t;

      public:
        static connectionPtr_t makeNew(boost::asio::io_service& _service)
        {
            connectionPtr_t newObject(new T(_service));
            return newObject;
        }

      public:
        void start()
        {
            m_started = true;

            if (m_outputMessageQueue.empty())
                readHeader();
            else
                writeMessage();
        }

        void stop()
        {
            if (!m_started)
                return;
            m_started = false;
            m_socket.close();
        }

        boost::asio::ip::tcp::socket& socket()
        {
            return m_socket;
        }

      private:
        void readHeader()
        {
            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_currentMsg.data(), CProtocolMessage::header_length),
                                    [this](boost::system::error_code ec, std::size_t /*length*/)
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
            });
        }

        void readBody()
        {
            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_currentMsg.body(), m_currentMsg.body_length()),
                                    [this](boost::system::error_code ec, std::size_t /*length*/)
                                    {
                if (!ec)
                {
                    LOG(MiscCommon::debug) << "Received from Agent: " << m_currentMsg.toString();
                    // processe recieved message
                    int nRes(0);
                    T* pThis = static_cast<T*>(this);
                    pThis->processMessage(m_currentMsg, nRes);
                    // Read next message
                    readHeader();
                }
                else
                {
                    stop();
                }
            });
        }

        void writeMessage()
        {
            while (!m_outputMessageQueue.empty())
            {
                CProtocolMessage msg = m_outputMessageQueue.front();
                m_outputMessageQueue.pop_front();
                boost::asio::async_write(m_socket,
                                         boost::asio::buffer(msg.data(), msg.length()),
                                         [this, &msg](boost::system::error_code ec, std::size_t /*_bytesTransferred*/)
                                         {
                    if (!ec)
                    {
                        LOG(MiscCommon::debug) << "Data successfully sent: " << msg.toString();
                    }
                    else
                    {
                        // stop();
                        LOG(MiscCommon::debug) << "Error sending data: " << msg.toString();
                    }
                });
            }

            // If there is no notghing to send, we return to read
            readHeader();
        }

      private:
        boost::asio::ip::tcp::socket m_socket;
        bool m_started;
        CProtocolMessage m_currentMsg;

        messageQueue_t m_outputMessageQueue;
    };
}

#endif /* defined(__DDS__Connection__) */
