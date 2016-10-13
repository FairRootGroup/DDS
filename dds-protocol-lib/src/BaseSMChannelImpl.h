// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BaseSMChannelImpl__
#define __DDS__BaseSMChannelImpl__
// STD
#include <chrono>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
// BOOST
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include <boost/asio.hpp>
#pragma clang diagnostic pop
// DDS
#include "ChannelMessageHandlersImpl.h"
#include "CommandAttachmentImpl.h"
#include "Logger.h"

#define BEGIN_SM_MSG_MAP(theClass)                                                        \
  public:                                                                                 \
    friend protocol_api::CBaseSMChannelImpl<theClass>;                                    \
    void processMessage(protocol_api::CProtocolMessage::protocolMessagePtr_t _currentMsg) \
    {                                                                                     \
        if (!m_started)                                                                   \
            return;                                                                       \
                                                                                          \
        using namespace dds;                                                              \
        using namespace dds::protocol_api;                                                \
        bool processed = true;                                                            \
        ECmdType currentCmd = static_cast<ECmdType>(_currentMsg->header().m_cmd);         \
                                                                                          \
        try                                                                               \
        {                                                                                 \
            switch (currentCmd)                                                           \
            {

#define SM_MESSAGE_HANDLER(msg, func)                                                                              \
    case msg:                                                                                                      \
    {                                                                                                              \
        typedef typename SCommandAttachmentImpl<msg>::ptr_t attahcmentPtr_t;                                       \
        attahcmentPtr_t attachmentPtr = SCommandAttachmentImpl<msg>::decode(_currentMsg);                          \
        processed = func(attachmentPtr);                                                                           \
        if (!processed)                                                                                            \
        {                                                                                                          \
            if (getNofMessageHandlers<msg>() == 0)                                                                 \
            {                                                                                                      \
                LOG(MiscCommon::error) << "The received message was not processed and has no registered handler: " \
                                       << _currentMsg->toString();                                                 \
            }                                                                                                      \
            else                                                                                                   \
            {                                                                                                      \
                dispatchMessageHandlers(msg, attachmentPtr, this);                                                 \
            }                                                                                                      \
        }                                                                                                          \
        break;                                                                                                     \
    }

#define END_SM_MSG_MAP()                                                                                         \
    default:                                                                                                     \
        LOG(MiscCommon::error) << "The received SM message doesn't have a handler: " << _currentMsg->toString(); \
        }                                                                                                        \
        }                                                                                                        \
        catch (std::exception & _e)                                                                              \
        {                                                                                                        \
            LOG(MiscCommon::error) << "SMChannel processMessage: " << _e.what();                                 \
        }                                                                                                        \
        }

namespace dds
{
    namespace protocol_api
    {
        template <class T>
        class CBaseSMChannelImpl : public boost::noncopyable,
                                   public CChannelMessageHandlersImpl,
                                   public std::enable_shared_from_this<T>
        {
            typedef std::deque<CProtocolMessage::protocolMessagePtr_t> protocolMessagePtrQueue_t;
            typedef std::shared_ptr<boost::interprocess::message_queue> messageQueuePtr_t;

          public:
            typedef std::shared_ptr<T> connectionPtr_t;

          protected:
            CBaseSMChannelImpl<T>(const std::string& _inputName, const std::string& _outputName)
                : CChannelMessageHandlersImpl()
                , m_started(false)
                , m_currentMsg(std::make_shared<CProtocolMessage>())
                , m_inputMessageQueueName(_inputName)
                , m_outputMessageQueueName(_outputName)
                , m_writeQueue()
                , m_mutexWriteBuffer()
                , m_writeBufferQueue()
            {
                try
                {
                    LOG(MiscCommon::info) << "Initializing message queue for shared memory channel";

                    static const unsigned int maxNofMessages = 100;
                    // Taking into account that maximum size of the string for the command is 2^16 plus some extra bytes
                    // for key size (128 bytes) and other.
                    static const unsigned int maxMessageSize = 65000;

                    m_transportIn.reset();
                    m_transportOut.reset();
                    m_transportIn =
                        std::make_shared<boost::interprocess::message_queue>(boost::interprocess::open_or_create,
                                                                             m_inputMessageQueueName.c_str(),
                                                                             maxNofMessages,
                                                                             maxMessageSize);
                    m_transportOut =
                        std::make_shared<boost::interprocess::message_queue>(boost::interprocess::open_or_create,
                                                                             m_outputMessageQueueName.c_str(),
                                                                             maxNofMessages,
                                                                             maxMessageSize);
                }
                catch (boost::interprocess::interprocess_exception& _e)
                {
                    LOG(MiscCommon::fatal) << "Can't initialize shared memory transport with input name " << _inputName
                                           << " and output name " << _outputName << ": " << _e.what();
                    m_transportIn.reset();
                    m_transportOut.reset();
                }
            }

          public:
            ~CBaseSMChannelImpl<T>()
            {
                LOG(MiscCommon::info) << "Shared memory channel destructor is called";
                stop();
            }

            static connectionPtr_t makeNew(const std::string& _inputName, const std::string& _outputName)
            {
                connectionPtr_t newObject(new T(_inputName, _outputName));
                return newObject;
            }

          public:
            void start()
            {
                if (m_transportIn == nullptr || m_transportOut == nullptr)
                {
                    LOG(MiscCommon::error)
                        << "Can't start shared memory channel because there was a problem creating message queues";
                    m_started = false;
                    return;
                }

                m_started = true;

                auto self(this->shared_from_this());
                m_io_service.post([this, self] {
                    try
                    {
                        readMessage();
                    }
                    catch (std::exception& ex)
                    {
                        LOG(MiscCommon::error) << "BaseSMChannelImpl can't read message: " << ex.what();
                    }
                });

                const int nConcurrentThreads(3);
                for (int x = 0; x < nConcurrentThreads; ++x)
                {
                    m_workerThreads.create_thread(boost::bind(&boost::asio::io_service::run, &(m_io_service)));
                }
            }

            void stop()
            {
                if (!m_started)
                    return;

                m_started = false;
                sendYourselfShutdown();

                m_io_service.stop();
                m_workerThreads.join_all();
            }

            void removeMessageQueue()
            {
                const bool inputRemoved = boost::interprocess::message_queue::remove(m_inputMessageQueueName.c_str());
                const bool outputRemoved = boost::interprocess::message_queue::remove(m_outputMessageQueueName.c_str());
                LOG(MiscCommon::info) << "Message queue " << m_inputMessageQueueName
                                      << " remove status: " << inputRemoved;
                LOG(MiscCommon::info) << "Message queue " << m_outputMessageQueueName
                                      << " remove status: " << outputRemoved;
            }

            template <ECmdType _cmd, class A>
            void pushMsg(const A& _attachment)
            {
                if (!m_started)
                {
                    LOG(MiscCommon::error) << "Skip pushing message. The channel was not started.";
                    return;
                }

                try
                {
                    CProtocolMessage::protocolMessagePtr_t msg = SCommandAttachmentImpl<_cmd>::encode(_attachment);

                    std::lock_guard<std::mutex> lock(m_mutexWriteBuffer);

                    // add the current message to the queue
                    if (cmdUNKNOWN != _cmd)
                        m_writeQueue.push_back(msg);

                    LOG(MiscCommon::debug) << "BaseSMChannelImpl pushMsg: WriteQueue size = " << m_writeQueue.size();
                }
                catch (std::exception& ex)
                {
                    LOG(MiscCommon::error) << "BaseSMChannelImpl can't push message: " << ex.what();
                }

                // process standard async writing
                auto self(this->shared_from_this());
                m_io_service.post([this, self] {
                    try
                    {
                        writeMessage();
                    }
                    catch (std::exception& ex)
                    {
                        LOG(MiscCommon::error) << "BaseSMChannelImpl can't write message: " << ex.what();
                    }
                });
            }

            template <ECmdType _cmd>
            void pushMsg()
            {
                SEmptyCmd cmd;
                pushMsg<_cmd>(cmd);
            }

          private:
            void sendYourselfShutdown()
            {
                // Send cmdSHUTDOWN with higher priority in order to stop read operation.
                SEmptyCmd cmd;
                CProtocolMessage::protocolMessagePtr_t msg = SCommandAttachmentImpl<cmdSHUTDOWN>::encode(cmd);
                m_transportIn->send(msg->data(), msg->length(), 1);
            }

            void readMessage()
            {
                try
                {
                    unsigned int priority;
                    boost::interprocess::message_queue::size_type receivedSize;

                    // We need to allocate the memory of the size equal to the maximum size of the message
                    m_currentMsg->resize(m_transportIn->get_max_msg_size());
                    m_transportIn->receive(
                        m_currentMsg->data(), m_transportIn->get_max_msg_size(), receivedSize, priority);

                    if (receivedSize < CProtocolMessage::header_length)
                    {
                        LOG(MiscCommon::debug) << "Received message: " << receivedSize << " bytes, expected at least"
                                               << CProtocolMessage::header_length << " bytes";
                    }
                    else
                    {
                        // Resize message data to the actually received bytes
                        m_currentMsg->resize(receivedSize);
                        if (m_currentMsg->decode_header())
                        {
                            ECmdType currentCmd = static_cast<ECmdType>(m_currentMsg->header().m_cmd);
                            if (currentCmd == cmdSHUTDOWN)
                            {
                                // Do not execute processBody which starts readMessage
                            }
                            else
                            {
                                // If the header is ok, process the body of the message
                                processBody(receivedSize - CProtocolMessage::header_length);
                            }
                        }
                        else
                        {
                            LOG(MiscCommon::error) << "BaseSMChannelImpl: error reading message header";
                        }
                    }
                }
                catch (boost::interprocess::interprocess_exception& ex)
                {
                    LOG(MiscCommon::error) << "BaseSMChannelImpl: error receiving message: " << ex.what() << "\n";
                }
            }

            void processBody(boost::interprocess::message_queue::size_type _bodySize)
            {
                if (_bodySize != m_currentMsg->body_length())
                {
                    LOG(MiscCommon::error) << "Received message BODY: " << _bodySize << " bytes, expected "
                                           << m_currentMsg->body_length();
                }
                else
                {
                    if (m_currentMsg->body_length() == 0)
                    {
                        LOG(MiscCommon::debug) << "Received message BODY no attachment: " << m_currentMsg->toString();
                    }
                    else
                    {
                        LOG(MiscCommon::debug) << "Received message BODY (" << _bodySize
                                               << " bytes): " << m_currentMsg->toString();
                    }

                    // process received message
                    T* pThis = static_cast<T*>(this);
                    pThis->processMessage(m_currentMsg);

                    // Read next message
                    m_currentMsg->clear();

                    auto self(this->shared_from_this());
                    m_io_service.post([this, self] {
                        try
                        {
                            readMessage();
                        }
                        catch (std::exception& ex)
                        {
                            LOG(MiscCommon::error) << "BaseSMChannelImpl can't read message: " << ex.what();
                        }
                    });
                }
            }

            void writeMessage()
            {
                {
                    std::lock_guard<std::mutex> lockWriteBuffer(m_mutexWriteBuffer);
                    if (!m_writeBufferQueue.empty())
                        return; // A write is in progress, don't start anything

                    if (m_writeQueue.empty())
                        return; // There is nothing to send.

                    m_writeBufferQueue.assign(m_writeQueue.begin(), m_writeQueue.end());
                    m_writeQueue.clear();
                }

                try
                {
                    for (auto msg : m_writeBufferQueue)
                    {
                        m_transportOut->send(msg->data(), msg->length(), 0);
                    }
                }
                catch (boost::interprocess::interprocess_exception& ex)
                {
                    LOG(MiscCommon::error) << "BaseSMChannelImpl: error sending message: " << ex.what();
                }

                // Lock the modification of the container
                {
                    std::lock_guard<std::mutex> lock(m_mutexWriteBuffer);
                    m_writeBufferQueue.clear();
                }
                // We might need to send more messages
                writeMessage();
            }

          protected:
            std::atomic<bool> m_started; ///< True if we were able to start the channel, False otherwise

          private:
            messageQueuePtr_t m_transportIn;                     ///< Input message queue, i.e. we read from this queue
            messageQueuePtr_t m_transportOut;                    ///< Output message queue, i.e. we write to this queue
            boost::asio::io_service m_io_service;                ///< IO service that is used as a thread pool
            boost::thread_group m_workerThreads;                 ///< Threads for IO service
            CProtocolMessage::protocolMessagePtr_t m_currentMsg; ///> Current message that we read and process

            std::string m_inputMessageQueueName;  ///< Input message queue name
            std::string m_outputMessageQueueName; ///< Output message queue name

            protocolMessagePtrQueue_t m_writeQueue; ///< Cache for the messages that we want to send

            std::mutex m_mutexWriteBuffer;
            protocolMessagePtrQueue_t m_writeBufferQueue;
        };
    }
}

#endif /* defined(__DDS__BaseSMChannelImpl__) */
