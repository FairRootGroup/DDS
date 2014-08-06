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
#include "MonitoringThread.h"

#define BEGIN_MSG_MAP(theClass)                                          \
  public:                                                                \
    friend CConnectionImpl<theClass>;                                    \
    void processMessage(const CProtocolMessage& _currentMsg, int& _nRes) \
    {                                                                    \
        dds::CMonitoringThread::instance().updateIdle();                 \
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

#define REGISTER_DEFAULT_ON_CONNECT_CALLBACKS \
    void onConnected()                        \
    {                                         \
    }                                         \
    void onFailedToConnect()                  \
    {                                         \
    }

#define REGISTER_DEFAULT_ON_DISCONNECT_CALLBACKS \
    void onRemoteEndDissconnected()              \
    {                                            \
    }

#define REGISTER_DEFAULT_ON_HEADER_READ_CALLBACKS \
    void onHeaderRead()                           \
    {                                             \
    }

#define REGISTER_ALL_DEFAULT_CALLBACKS                                             \
    REGISTER_DEFAULT_ON_CONNECT_CALLBACKS REGISTER_DEFAULT_ON_DISCONNECT_CALLBACKS \
        REGISTER_DEFAULT_ON_HEADER_READ_CALLBACKS

namespace dds
{
    template <class T>
    class CConnectionImpl : public boost::noncopyable
    {
      protected:
        CConnectionImpl<T>(boost::asio::io_service& _service)
            : m_io_service(_service)
            , m_socket(_service)
            , m_started(false)
            , m_currentMsg()
            , m_outputMessageQueue()
        {
        }

      public:
        ~CConnectionImpl<T>()
        {
            stop();
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
        void connect(boost::asio::ip::tcp::resolver::iterator _endpoint_iterator)
        {
            doConnect(_endpoint_iterator);
        }

        void start()
        {
            if (m_started)
                return;

            m_started = true;
            readHeader();
        }

        void stop()
        {
            if (!m_started)
                return;
            m_started = false;
            close();
        }

        boost::asio::ip::tcp::socket& socket()
        {
            return m_socket;
        }

        void pushMsg(const CProtocolMessage& _msg)
        {
            m_io_service.post([this, _msg]()
                              {
                                  bool write_in_progress = !m_outputMessageQueue.empty();
                                  m_outputMessageQueue.push_back(_msg);
                                  if (!write_in_progress)
                                  {
                                      writeMessages();
                                  }
                              });
        }

        template <ECmdType _cmd>
        void pushMsg()
        {
            CProtocolMessage msg;
            msg.encode<_cmd>();
            pushMsg(msg);
        }

      private:
        void doConnect(boost::asio::ip::tcp::resolver::iterator _endpoint_iterator)
        {
            boost::asio::async_connect(m_socket,
                                       _endpoint_iterator,
                                       [this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
                                       {
                if (!ec)
                {
                    // give a chance to child to execute something
                    T* pThis = static_cast<T*>(this);
                    pThis->onConnected();

                    // start the communication channel
                    start();
                }
                else
                {
                    // give a chance to child to execute something
                    T* pThis = static_cast<T*>(this);
                    pThis->onFailedToConnect();
                }
            });
        }

        void readHeader()
        {
            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_currentMsg.data(), CProtocolMessage::header_length),
                                    [this](boost::system::error_code ec, std::size_t length)
                                    {
                LOG(MiscCommon::debug) << "readHeader received: " << length << " bytes, expected "
                                       << CProtocolMessage::header_length << ", from "
                                       << socket().remote_endpoint().address().to_string();
                if (!ec && m_currentMsg.decode_header())
                {
                    // give a chance to child to execute something
                    T* pThis = static_cast<T*>(this);
                    pThis->onHeaderRead();

                    // If the header is ok, receive the body of the message
                    readBody();
                }
                else if ((boost::asio::error::eof == ec) || (boost::asio::error::connection_reset == ec))
                {
                    LOG(MiscCommon::debug) << "The session was disconnected by the remote end";
                    // give a chance to child to execute something
                    T* pThis = static_cast<T*>(this);
                    pThis->onRemoteEndDissconnected();
                }
                else
                {
                    if (m_started)
                        LOG(MiscCommon::error) << "Error reading message header: " << ec.message();
                    else
                        LOG(MiscCommon::info)
                            << "The stop signal is received, aborting current operation and closing the connection."
                            << ec.message();

                    stop();
                }
            });
        }

        void readBody()
        {
            if (m_currentMsg.body_length() == 0)
            {
                LOG(MiscCommon::debug) << "readBody: the message has no attachment: " << m_currentMsg.toString();
                // process received message
                int nRes(0);
                T* pThis = static_cast<T*>(this);
                pThis->processMessage(m_currentMsg, nRes);
                // Read next message
                m_currentMsg.clear();
                readHeader();
                return;
            }

            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_currentMsg.body(), m_currentMsg.body_length()),
                                    [this](boost::system::error_code ec, std::size_t length)
                                    {
                if (!ec)
                {
                    LOG(MiscCommon::debug) << "Received from Agent: " << m_currentMsg.toString();
                    // process received message
                    int nRes(0);
                    T* pThis = static_cast<T*>(this);
                    pThis->processMessage(m_currentMsg, nRes);
                    // Read next message
                    m_currentMsg.clear();
                    readHeader();
                }
                else if ((boost::asio::error::eof == ec) || (boost::asio::error::connection_reset == ec))
                {
                    LOG(MiscCommon::debug) << "The session was disconnected by the remote end";
                    // give a chance to child to execute something
                    T* pThis = static_cast<T*>(this);
                    pThis->onRemoteEndDissconnected();
                }
                else
                {
                    // don't show error if service is closed
                    if (m_started)
                        LOG(MiscCommon::error) << "Error reading message body: " << ec.message();
                    else
                        LOG(MiscCommon::info)
                            << "The stop signal is received, aborting current operation and closing the connection."
                            << ec.message();
                    stop();
                }
            });
        }

        void writeMessages()
        {
            LOG(MiscCommon::debug) << "Sending message: " << m_outputMessageQueue.front().toString();
            boost::asio::async_write(
                m_socket,
                boost::asio::buffer(m_outputMessageQueue.front().data(), m_outputMessageQueue.front().length()),
                [this](boost::system::error_code ec, std::size_t /*_bytesTransferred*/)
                {
                    if (!ec)
                    {
                        LOG(MiscCommon::debug) << "Data successfully sent";
                        m_outputMessageQueue.pop_front();
                        if (!m_outputMessageQueue.empty())
                        {
                            writeMessages();
                        }
                    }
                    else if ((boost::asio::error::eof == ec) || (boost::asio::error::connection_reset == ec))
                    {
                        LOG(MiscCommon::debug) << "The session was disconnected by the remote end";
                        // give a chance to child to execute something
                        T* pThis = static_cast<T*>(this);
                        pThis->onRemoteEndDissconnected();
                    }
                    else
                    {
                        // don't show error if service is closed
                        if (m_started)
                            LOG(MiscCommon::error) << "Error sending data: " << ec.message();
                        else
                            LOG(MiscCommon::info)
                                << "The stop signal is received, aborting current operation and closing the connection."
                                << ec.message();
                        stop();
                    }
                });
        }

      private:
        void close()
        {
            m_io_service.post([this]()
                              { m_socket.close(); });
        }

      private:
        boost::asio::io_service& m_io_service;
        boost::asio::ip::tcp::socket m_socket;
        bool m_started;
        CProtocolMessage m_currentMsg;
        messageQueue_t m_outputMessageQueue;
    };
}

#endif /* defined(__DDS__Connection__) */
