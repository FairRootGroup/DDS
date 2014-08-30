// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__Connection__
#define __DDS__Connection__
// STD
#include <iostream>
#include <deque>
#include <map>
// BOOST
#include "boost/noncopyable.hpp"
#include "boost/asio.hpp"
// DDS
#include "ProtocolMessage.h"
#include "ProtocolCommands.h"
#include "Logger.h"
#include "MonitoringThread.h"

#define BEGIN_MSG_MAP(theClass)                                             \
  public:                                                                   \
    friend CConnectionImpl<theClass>;                                       \
    void processMessage(CProtocolMessage::protocolMessagePtr_t _currentMsg) \
    {                                                                       \
        dds::CMonitoringThread::instance().updateIdle();                    \
        bool processed = true;                                              \
        switch (_currentMsg->header().m_cmd)                                \
        {

#define MESSAGE_HANDLER(msg, func)     \
    case msg:                          \
        processed = func(_currentMsg); \
        break;

#define END_MSG_MAP()                                                                                              \
    default:                                                                                                       \
        LOG(MiscCommon::error) << "The received message doesn't have a handler: " << _currentMsg->toString();      \
        }                                                                                                          \
        if (!processed)                                                                                            \
        {                                                                                                          \
            ECmdType currentCmd = static_cast<ECmdType>(_currentMsg->header().m_cmd);                              \
            if (m_registeredMessageHandlers.count(currentCmd) == 0)                                                \
            {                                                                                                      \
                LOG(MiscCommon::error) << "The received message was not processed and has no registered handler: " \
                                       << _currentMsg->toString();                                                 \
            }                                                                                                      \
            else                                                                                                   \
            {                                                                                                      \
                auto functions = m_registeredMessageHandlers.equal_range(currentCmd);                              \
                for (auto it = functions.first; it != functions.second; ++it)                                      \
                {                                                                                                  \
                    it->second(_currentMsg, this);                                                                 \
                }                                                                                                  \
            }                                                                                                      \
        }                                                                                                          \
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
        typedef std::function<bool(CProtocolMessage::protocolMessagePtr_t, T*)> handlerFunction_t;
        typedef std::function<void(T*)> handlerDisconnectEventFunction_t;

      protected:
        CConnectionImpl<T>(boost::asio::io_service& _service)
            : m_io_service(_service)
            , m_socket(_service)
            , m_started(false)
            , m_currentMsg(std::make_shared<CProtocolMessage>())
        {
        }

      public:
        ~CConnectionImpl<T>()
        {
            stop();
        }

      public:
        typedef std::shared_ptr<T> connectionPtr_t;
        typedef std::weak_ptr<T> weakConnectionPtr_t;
        typedef std::vector<connectionPtr_t> connectionPtrVector_t;
        typedef std::vector<weakConnectionPtr_t> weakConnectionPtrVector_t;

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

        void pushMsg(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            // it can be called from multiple IO threads
            std::lock_guard<std::mutex> lock(m_mutex);
            m_writeMsg.push_back(_msg);

            writeMessage(_msg);
        }

        template <ECmdType _cmd>
        void pushMsg()
        {
            CProtocolMessage::protocolMessagePtr_t msg = std::make_shared<CProtocolMessage>();
            msg->encode<_cmd>();
            pushMsg(msg);
        }

        void syncPushMsg(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            syncWriteMessage(_msg);
        }

        template <ECmdType _cmd>
        void syncPushMsg()
        {
            CProtocolMessage::protocolMessagePtr_t msg = std::make_shared<CProtocolMessage>();
            msg->encode<_cmd>();
            syncPushMsg(msg);
        }

        void registerMessageHandler(ECmdType _type, handlerFunction_t _handler)
        {
            m_registeredMessageHandlers.insert(std::pair<ECmdType, handlerFunction_t>(_type, _handler));
        }

        void registerDissconnectEventHandler(handlerDisconnectEventFunction_t _handler)
        {
            m_dissconnectEventHandler = _handler;
        }

        bool started()
        {
            return m_started;
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
                    // give a chance child to execute something
                    T* pThis = static_cast<T*>(this);
                    pThis->onConnected();

                    // start the communication channel
                    start();

                    // Prepare a hand shake message
                    SVersionCmd cmd;
                    CProtocolMessage::protocolMessagePtr_t msg = std::make_shared<CProtocolMessage>();
                    msg->encodeWithAttachment<cmdHANDSHAKE>(cmd);
                    pushMsg(msg);
                }
                else
                {
                    // give a chance child to execute something
                    T* pThis = static_cast<T*>(this);
                    pThis->onFailedToConnect();
                }
            });
        }

        void readHeader()
        {
            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_currentMsg->data(), CProtocolMessage::header_length),
                                    [this](boost::system::error_code ec, std::size_t length)
                                    {
                if (!ec)
                {
                    LOG(MiscCommon::debug) << "readHeader received: " << length << " bytes, expected "
                                           << CProtocolMessage::header_length << ", from "
                                           << socket().remote_endpoint().address().to_string();
                }
                if (!ec && m_currentMsg->decode_header())
                {
                    // give a chance to child to execute something
                    T* pThis = static_cast<T*>(this);
                    pThis->onHeaderRead();

                    // If the header is ok, receive the body of the message
                    readBody();
                }
                else if ((boost::asio::error::eof == ec) || (boost::asio::error::connection_reset == ec))
                {
                    onDissconnect();
                }
                else
                {
                    if (m_started)
                        LOG(MiscCommon::error) << "Error reading message header: " << ec.message();
                    else
                        LOG(MiscCommon::info)
                            << "The stop signal is received, aborting current operation and closing the connection: "
                            << ec.message();

                    stop();
                }
            });
        }

        void readBody()
        {
            if (m_currentMsg->body_length() == 0)
            {
                LOG(MiscCommon::debug) << "readBody: the message has no attachment: " << m_currentMsg->toString();
                // process received message
                T* pThis = static_cast<T*>(this);
                pThis->processMessage(m_currentMsg);
                // Read next message
                m_currentMsg->clear();
                readHeader();
                return;
            }

            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_currentMsg->body(), m_currentMsg->body_length()),
                                    [this](boost::system::error_code ec, std::size_t length)
                                    {
                if (!ec)
                {
                    LOG(MiscCommon::debug) << "Received (" << length << ") from Agent: " << m_currentMsg->toString();
                    // process received message
                    T* pThis = static_cast<T*>(this);
                    pThis->processMessage(m_currentMsg);
                    // Read next message
                    m_currentMsg->clear();
                    readHeader();
                }
                else if ((boost::asio::error::eof == ec) || (boost::asio::error::connection_reset == ec))
                {
                    onDissconnect();
                }
                else
                {
                    // don't show error if service is closed
                    if (m_started)
                        LOG(MiscCommon::error) << "Error reading message body: " << ec.message();
                    else
                        LOG(MiscCommon::info)
                            << "The stop signal is received, aborting current operation and closing the connection: "
                            << ec.message();
                    stop();
                }
            });
        }

      private:
        void writeMessage(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            LOG(MiscCommon::debug) << "Sending message: " << _msg->toString();
            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(_msg->data(), _msg->length()),
                                     [this](boost::system::error_code _ec, std::size_t _bytesTransferred)
                                     {
                writeHandler(_ec, _bytesTransferred);
            });
        }

        void syncWriteMessage(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            LOG(MiscCommon::debug) << "Sending message: " << _msg->toString();
            boost::system::error_code ec;
            size_t bytesTransfered = boost::asio::write(
                m_socket, boost::asio::buffer(_msg->data(), _msg->length()), boost::asio::transfer_all(), ec);

            writeHandler(ec, bytesTransfered);
        }

        void writeHandler(boost::system::error_code _ec, std::size_t _bytesTransferred)
        {
            if (!_ec)
            {
                LOG(MiscCommon::debug) << "Data successfully sent: " << _bytesTransferred << " bytes transferred.";
            }
            else if ((boost::asio::error::eof == _ec) || (boost::asio::error::connection_reset == _ec))
            {
                onDissconnect();
            }
            else
            {
                // don't show error if service is closed
                if (m_started)
                    LOG(MiscCommon::error) << "Error sending data: " << _ec.message();
                else
                    LOG(MiscCommon::info)
                        << "The stop signal is received, aborting current operation and closing the connection: "
                        << _ec.message();
                stop();
            }
        }
        void onDissconnect()
        {
            LOG(MiscCommon::debug) << "The session was disconnected by the remote end";
            // give a chance to child to execute something
            T* pThis = static_cast<T*>(this);
            pThis->onRemoteEndDissconnected();

            // Call external event handler
            if (m_dissconnectEventHandler)
                m_dissconnectEventHandler(pThis);
        }

      private:
        void close()
        {
            m_socket.close();
        }

      private:
        boost::asio::io_service& m_io_service;
        boost::asio::ip::tcp::socket m_socket;
        bool m_started;
        CProtocolMessage::protocolMessagePtr_t m_currentMsg;
        CProtocolMessage::protocolMessagePtrVector_t m_writeMsg;
        std::mutex m_mutex;

      protected:
        std::multimap<ECmdType, handlerFunction_t> m_registeredMessageHandlers;
        handlerDisconnectEventFunction_t m_dissconnectEventHandler;
    };
}

#endif /* defined(__DDS__Connection__) */
