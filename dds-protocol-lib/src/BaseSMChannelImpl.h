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
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <boost/asio.hpp>
#pragma clang diagnostic pop
// DDS
#include "ChannelEventHandlersImpl.h"
#include "ChannelMessageHandlersImpl.h"
#include "CommandAttachmentImpl.h"
#include "Logger.h"

// Either raw message or command based processing can be used at a time
// Command based message processing
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
        SSenderInfo sender;                                                               \
        sender.m_ID = _currentMsg->header().m_ID;                                         \
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
        processed = func(attachmentPtr, sender);                                                                   \
        if (!processed)                                                                                            \
        {                                                                                                          \
            if (!handlerExists(msg))                                                                               \
            {                                                                                                      \
                LOG(MiscCommon::error) << "The received message was not processed and has no registered handler: " \
                                       << _currentMsg->toString();                                                 \
            }                                                                                                      \
            else                                                                                                   \
            {                                                                                                      \
                dispatchHandlers(msg, sender, attachmentPtr);                                                      \
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

// Raw message processing
#define SM_RAW_MESSAGE_HANDLER(theClass, func)                                                                         \
  public:                                                                                                              \
    friend protocol_api::CBaseSMChannelImpl<theClass>;                                                                 \
    void processMessage(protocol_api::CProtocolMessage::protocolMessagePtr_t _currentMsg)                              \
    {                                                                                                                  \
        if (!m_started)                                                                                                \
            return;                                                                                                    \
                                                                                                                       \
        using namespace dds;                                                                                           \
        using namespace dds::protocol_api;                                                                             \
        bool processed = true;                                                                                         \
        SSenderInfo sender;                                                                                            \
        sender.m_ID = _currentMsg->header().m_ID;                                                                      \
        try                                                                                                            \
        {                                                                                                              \
            processed = func(_currentMsg, sender);                                                                     \
            if (!processed)                                                                                            \
            {                                                                                                          \
                if (!handlerExists(ECmdType::cmdRAW_MSG))                                                              \
                {                                                                                                      \
                    LOG(MiscCommon::error) << "The received message was not processed and has no registered handler: " \
                                           << _currentMsg->toString();                                                 \
                }                                                                                                      \
                else                                                                                                   \
                {                                                                                                      \
                    dispatchHandlers(ECmdType::cmdRAW_MSG, sender, _currentMsg);                                       \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
        catch (std::exception & _e)                                                                                    \
        {                                                                                                              \
            LOG(MiscCommon::error) << "SMChannel processMessage: " << _e.what();                                       \
        }                                                                                                              \
    }

namespace dds
{
    namespace protocol_api
    {
        enum class EMQOpenType
        {
            CreateOnly,
            OpenOrCreate,
            OpenOnly
        };

        template <class T>
        class CBaseSMChannelImpl : public boost::noncopyable,
                                   public CChannelEventHandlersImpl,
                                   public CChannelMessageHandlersImpl,
                                   public std::enable_shared_from_this<T>
        {
            struct SProtocolMessageInfo
            {
                SProtocolMessageInfo(uint64_t _protocolHeaderID, CProtocolMessage::protocolMessagePtr_t _msg)
                    : m_protocolHeaderID(_protocolHeaderID)
                    , m_msg(_msg)
                {
                }

                uint64_t m_protocolHeaderID;
                CProtocolMessage::protocolMessagePtr_t m_msg;
            };

            typedef std::deque<SProtocolMessageInfo> protocolMessagePtrQueue_t;

            typedef std::shared_ptr<boost::interprocess::message_queue> messageQueuePtr_t;

            struct SMessageQueueInfo
            {
                std::string m_name;     ///< Message queue name
                EMQOpenType m_openType; ///< Message queue open type
                messageQueuePtr_t m_mq; ///< Message queue
            };

            typedef std::map<uint64_t, SMessageQueueInfo> messageQueueMap_t;

          public:
            typedef std::shared_ptr<T> connectionPtr_t;
            typedef std::weak_ptr<T> weakConnectionPtr_t;

            // Both are needed because unqualified name lookup terminates at the first scope that has anything with the
            // right name
            DDS_DECLARE_EVENT_HANDLER_CLASS(CChannelEventHandlersImpl)
            DDS_DECLARE_EVENT_HANDLER_CLASS(CChannelMessageHandlersImpl)

          protected:
            CBaseSMChannelImpl<T>(const std::string& _inputName,
                                  const std::string& _outputName,
                                  uint64_t _protocolHeaderID,
                                  EMQOpenType _inputOpenType = EMQOpenType::OpenOrCreate,
                                  EMQOpenType _outputOpenType = EMQOpenType::OpenOrCreate)
                : CChannelMessageHandlersImpl()
                , m_started(false)
                , m_protocolHeaderID(_protocolHeaderID)
                , m_workerThreads()
                , m_currentMsg(std::make_shared<CProtocolMessage>())
                , m_writeQueue()
                , m_mutexWriteBuffer()
                , m_writeBufferQueue()
                , m_mutexTransportOut()
            {
                // Input transport
                m_transportIn.m_name = _inputName;
                m_transportIn.m_openType = _inputOpenType;
                // Output transport - default output transport initialized with protocol header ID
                SMessageQueueInfo info;
                info.m_name = _outputName;
                info.m_openType = _outputOpenType;
                m_transportOut.emplace(_protocolHeaderID, info);

                createMessageQueue();
            }

          public:
            ~CBaseSMChannelImpl<T>()
            {
                LOG(MiscCommon::info) << "Shared memory channel destructor is called";
                stop();
            }

            static connectionPtr_t makeNew(const std::string& _inputName,
                                           const std::string& _outputName,
                                           uint64_t _ProtocolHeaderID)
            {
                connectionPtr_t newObject(new T(_inputName, _outputName, _ProtocolHeaderID));
                return newObject;
            }

          private:
            void createMessageQueue()
            {
                LOG(MiscCommon::info) << "Initializing message queue for shared memory channel";

                m_transportIn.m_mq.reset();
                m_transportIn.m_mq = createMessageQueue(m_transportIn.m_name.c_str(), m_transportIn.m_openType);

                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                    for (auto& v : m_transportOut)
                    {
                        SMessageQueueInfo& info = v.second;
                        info.m_mq.reset();
                        info.m_mq = createMessageQueue(info.m_name.c_str(), info.m_openType);
                    }
                }
            }

            messageQueuePtr_t createMessageQueue(const std::string& _name, EMQOpenType _openType)
            {
                static const unsigned int maxNofMessages = 100;
                // Taking into account that maximum size of the string for the command is 2^16 plus some extra bytes
                // for key size (128 bytes) and other.
                static const unsigned int maxMessageSize = 65000;

                try
                {
                    switch (_openType)
                    {
                        case EMQOpenType::OpenOrCreate:
                            return std::make_shared<boost::interprocess::message_queue>(
                                boost::interprocess::open_or_create, _name.c_str(), maxNofMessages, maxMessageSize);
                        case EMQOpenType::CreateOnly:
                            return std::make_shared<boost::interprocess::message_queue>(
                                boost::interprocess::create_only, _name.c_str(), maxNofMessages, maxMessageSize);
                        case EMQOpenType::OpenOnly:
                            return std::make_shared<boost::interprocess::message_queue>(boost::interprocess::open_only,
                                                                                        _name.c_str());
                    }
                }
                catch (boost::interprocess::interprocess_exception& _e)
                {
                    LOG(MiscCommon::error)
                        << "Can't initialize shared memory transport with name " << _name << ": " << _e.what();
                    return nullptr;
                }
            }

          public:
            void addOutput(uint64_t _protocolHeaderID,
                           const std::string& _name,
                           EMQOpenType _openType = EMQOpenType::OpenOnly)
            {
                SMessageQueueInfo info;
                info.m_name = _name;
                info.m_openType = _openType;
                info.m_mq = createMessageQueue(_name, _openType);

                if (info.m_mq != nullptr)
                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                    m_transportOut.emplace(_protocolHeaderID, info);
                }
                else
                {
                    LOG(MiscCommon::error)
                        << "Can't add shared memory channel output with protocol header ID: " << _protocolHeaderID
                        << " name: " << _name;
                }
            }

            void reinit()
            {
                if (!m_started)
                    return;

                LOG(MiscCommon::info) << "Reinitializing shared memory channel...";

                stop();
                removeMessageQueue();
                createMessageQueue();
                {
                    // Clear message buffers
                    std::lock_guard<std::mutex> lockWriteBuffer(m_mutexWriteBuffer);
                    m_writeBufferQueue.clear();
                    m_writeQueue.clear();
                }
                start();
            }

            void start(bool _block = false)
            {
                // Check that all message queues were succesfully created
                bool queuesCreated = m_transportIn.m_mq != nullptr;
                if (queuesCreated)
                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);

                    for (const auto& v : m_transportOut)
                    {
                        if (v.second.m_mq == nullptr)
                        {
                            queuesCreated = false;
                            break;
                        }
                    }
                }
                if (!queuesCreated)
                {
                    LOG(MiscCommon::error)
                        << "Can't start shared memory channel because there was a problem creating message queues";
                    m_started = false;
                    return;
                }
                //

                m_started = true;

                m_io_service.reset();

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
                m_workerThreads = std::make_shared<boost::thread_group>();
                for (int x = 0; x < nConcurrentThreads; ++x)
                {
                    m_workerThreads->create_thread(boost::bind(&boost::asio::io_service::run, &(m_io_service)));
                }

                if (_block)
                {
                    m_workerThreads->join_all();
                }
            }

            void stop()
            {
                if (!m_started)
                    return;

                m_started = false;
                sendYourselfShutdown();

                m_io_service.stop();
                m_workerThreads->join_all();
                m_workerThreads.reset();
            }

            void removeMessageQueue()
            {
                const bool status = boost::interprocess::message_queue::remove(m_transportIn.m_name.c_str());
                LOG(MiscCommon::info) << "Message queue " << m_transportIn.m_name << " remove status: " << status;
                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                    for (const auto& v : m_transportOut)
                    {
                        const SMessageQueueInfo& info = v.second;
                        const bool status = boost::interprocess::message_queue::remove(info.m_name.c_str());
                        LOG(MiscCommon::info) << "Message queue " << info.m_name << " remove status: " << status;
                    }
                }
            }

            void pushMsg(CProtocolMessage::protocolMessagePtr_t _msg, ECmdType _cmd, uint64_t _protocolHeaderID = 0)
            {
                if (!m_started)
                {
                    LOG(MiscCommon::error) << "Skip pushing message. The channel was not started.";
                    return;
                }

                try
                {
                    std::lock_guard<std::mutex> lock(m_mutexWriteBuffer);

                    // add the current message to the queue
                    if (cmdUNKNOWN != _cmd)
                        m_writeQueue.push_back(SProtocolMessageInfo(protocolHeaderID(_protocolHeaderID), _msg));

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

            template <ECmdType _cmd, class A>
            void pushMsg(const A& _attachment, uint64_t _protocolHeaderID = 0)
            {
                try
                {
                    uint64_t headerID = protocolHeaderID(_protocolHeaderID);
                    CProtocolMessage::protocolMessagePtr_t msg =
                        SCommandAttachmentImpl<_cmd>::encode(_attachment, headerID);
                    pushMsg(msg, _cmd, headerID);
                }
                catch (std::exception& ex)
                {
                    LOG(MiscCommon::error) << "BaseSMChannelImpl can't push message: " << ex.what();
                }
            }

            template <ECmdType _cmd>
            void pushMsg(uint64_t _protocolHeaderID = 0)
            {
                SEmptyCmd cmd;
                pushMsg<_cmd>(cmd, _protocolHeaderID);
            }

          private:
            uint64_t protocolHeaderID(uint64_t _protocolHeaderID) const
            {
                return (_protocolHeaderID == 0) ? m_protocolHeaderID : _protocolHeaderID;
            }

            void sendYourselfShutdown()
            {
                // Send cmdSHUTDOWN with higher priority in order to stop read operation.
                SEmptyCmd cmd;
                CProtocolMessage::protocolMessagePtr_t msg =
                    SCommandAttachmentImpl<cmdSHUTDOWN>::encode(cmd, m_protocolHeaderID);
                m_transportIn.m_mq->send(msg->data(), msg->length(), 1);
            }

            void readMessage()
            {
                try
                {
                    unsigned int priority;
                    boost::interprocess::message_queue::size_type receivedSize;

                    // We need to allocate the memory of the size equal to the maximum size of the message
                    m_currentMsg->resize(m_transportIn.m_mq->get_max_msg_size());
                    m_transportIn.m_mq->receive(
                        m_currentMsg->data(), m_transportIn.m_mq->get_max_msg_size(), receivedSize, priority);

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
                            // If the header is ok, process the body of the message
                            processBody(receivedSize - CProtocolMessage::header_length);
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
                    LOG(MiscCommon::error)
                        << "Received message BODY: " << _bodySize << " bytes, expected " << m_currentMsg->body_length();
                }
                else
                {
                    if (m_currentMsg->body_length() == 0)
                    {
                        LOG(MiscCommon::debug) << "Received message BODY no attachment: " << m_currentMsg->toString();
                    }
                    else
                    {
                        LOG(MiscCommon::debug)
                            << "Received message BODY (" << _bodySize << " bytes): " << m_currentMsg->toString();
                    }

                    // process received message
                    T* pThis = static_cast<T*>(this);
                    pThis->processMessage(m_currentMsg);

                    ECmdType currentCmd = static_cast<ECmdType>(m_currentMsg->header().m_cmd);

                    // Read next message
                    m_currentMsg = std::make_shared<CProtocolMessage>();

                    // Do not start readMessage if cmdSHUTDOWN was sent
                    if (currentCmd != cmdSHUTDOWN)
                    {
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
                    for (auto& msg : m_writeBufferQueue)
                    {
                        messageQueuePtr_t mq = nullptr;
                        bool exists = false;
                        {
                            std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                            auto it = m_transportOut.find(msg.m_protocolHeaderID);
                            exists = (it != m_transportOut.end());
                            mq = it->second.m_mq;
                        }

                        if (exists && mq != nullptr)
                        {
                            mq->send(msg.m_msg->data(), msg.m_msg->length(), 0);
                        }
                        else
                        {
                            LOG(MiscCommon::error) << "Can't find output transport with protocol header ID "
                                                   << msg.m_protocolHeaderID << ". Write message failed.";
                        }
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
            uint64_t m_protocolHeaderID;

          private:
            SMessageQueueInfo m_transportIn;  ///< Input message queue, i.e. we read from this queue
            messageQueueMap_t m_transportOut; ///< Map of output message queues, i.e. we write to this queues
            std::mutex m_mutexTransportOut;
            boost::asio::io_service m_io_service;                 ///< IO service that is used as a thread pool
            std::shared_ptr<boost::thread_group> m_workerThreads; ///< Threads for IO service
            CProtocolMessage::protocolMessagePtr_t m_currentMsg;  ///> Current message that we read and process

            protocolMessagePtrQueue_t m_writeQueue; ///< Cache for the messages that we want to send

            std::mutex m_mutexWriteBuffer;
            protocolMessagePtrQueue_t m_writeBufferQueue;
        };
    }
}

#endif /* defined(__DDS__BaseSMChannelImpl__) */
