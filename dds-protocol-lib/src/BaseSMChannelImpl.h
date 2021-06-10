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
#include <boost/asio.hpp>
#include <boost/date_time.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
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
        bool processed = func(attachmentPtr, sender);                                                              \
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

#define SM_MESSAGE_HANDLER_DISPATCH(msg)                                                                         \
    case msg:                                                                                                    \
    {                                                                                                            \
        typedef typename SCommandAttachmentImpl<msg>::ptr_t attahcmentPtr_t;                                     \
        attahcmentPtr_t attachmentPtr = SCommandAttachmentImpl<msg>::decode(_currentMsg);                        \
        LOG(MiscCommon::debug) << "Dispatching " << g_cmdToString[msg];                                          \
        if (!handlerExists(msg))                                                                                 \
        {                                                                                                        \
            LOG(MiscCommon::error) << "The received message can't be dispatched, it has no registered handler: " \
                                   << _currentMsg->toString();                                                   \
        }                                                                                                        \
        else                                                                                                     \
        {                                                                                                        \
            dispatchHandlers<>(msg, sender, attachmentPtr);                                                      \
        }                                                                                                        \
        break;                                                                                                   \
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
        SSenderInfo sender;                                                                                            \
        sender.m_ID = _currentMsg->header().m_ID;                                                                      \
        try                                                                                                            \
        {                                                                                                              \
            bool processed = func(_currentMsg, sender);                                                                \
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
                SProtocolMessageInfo(uint64_t _outputID, CProtocolMessage::protocolMessagePtr_t _msg)
                    : m_outputID(_outputID)
                    , m_msg(_msg)
                {
                }

                uint64_t m_outputID;
                CProtocolMessage::protocolMessagePtr_t m_msg;
            };

            typedef std::deque<SProtocolMessageInfo> protocolMessagePtrQueue_t;

            typedef std::shared_ptr<boost::interprocess::message_queue> messageQueuePtr_t;

            struct SMessageQueueInfo
            {
                using Container_t = std::vector<SMessageQueueInfo>;

                std::string m_name;     ///< Message queue name
                EMQOpenType m_openType; ///< Message queue open type
                messageQueuePtr_t m_mq; ///< Message queue
            };

            struct SMessageOutputBuffer
            {
                using Ptr_t = std::shared_ptr<SMessageOutputBuffer>;
                using Container_t = std::map<uint64_t, Ptr_t>;

                SMessageQueueInfo m_info;

                protocolMessagePtrQueue_t m_writeQueue; ///< Cache for the messages that we want to send
                std::mutex m_mutexWriteBuffer;
                protocolMessagePtrQueue_t m_writeBufferQueue;
                std::atomic<bool> m_drainWriteQueue{ false };
            };

          public:
            typedef std::shared_ptr<T> connectionPtr_t;
            typedef std::weak_ptr<T> weakConnectionPtr_t;

            // Both are needed because unqualified name lookup terminates at the first scope that has anything with the
            // right name
            DDS_DECLARE_EVENT_HANDLER_CLASS(CChannelEventHandlersImpl)
            DDS_DECLARE_EVENT_HANDLER_CLASS(CChannelMessageHandlersImpl)

          protected:
            CBaseSMChannelImpl<T>(boost::asio::io_context& _service,
                                  const std::string& _inputName,
                                  const std::string& _outputName,
                                  uint64_t _protocolHeaderID,
                                  EMQOpenType _inputOpenType,
                                  EMQOpenType _outputOpenType)
                : CChannelMessageHandlersImpl()
                , m_isShuttingDown(false)
                , m_started(false)
                , m_protocolHeaderID(_protocolHeaderID)
                , m_ioContext(_service)
            {
                defaultInit({ _inputName }, _outputName, _inputOpenType, _outputOpenType);
            }

            CBaseSMChannelImpl<T>(boost::asio::io_context& _service,
                                  const std::vector<std::string>& _inputNames,
                                  const std::string& _outputName,
                                  uint64_t _protocolHeaderID,
                                  EMQOpenType _inputOpenType,
                                  EMQOpenType _outputOpenType)
                : CChannelMessageHandlersImpl()
                , m_isShuttingDown(false)
                , m_started(false)
                , m_protocolHeaderID(_protocolHeaderID)
                , m_ioContext(_service)
            {
                defaultInit(_inputNames, _outputName, _inputOpenType, _outputOpenType);
            }

            void defaultInit(const std::vector<std::string> _inputNames,
                             const std::string& _outputName,
                             EMQOpenType _inputOpenType,
                             EMQOpenType _outputOpenType)
            {
                for (const auto& v : _inputNames)
                {
                    SMessageQueueInfo inInfo;
                    inInfo.m_name = v;
                    inInfo.m_openType = _inputOpenType;
                    m_transportIn.push_back(inInfo);
                }

                // Output transport - default output transport initialized with protocol header ID
                auto buffer = std::make_shared<SMessageOutputBuffer>();
                buffer->m_info.m_name = _outputName;
                buffer->m_info.m_openType = _outputOpenType;
                m_outputBuffers.emplace(0, buffer);

                createMessageQueue();

                LOG(MiscCommon::info) << "SM: New channel: inputName=" << m_transportIn.front().m_name
                                      << " outputName=" << _outputName << " protocolHeaderID=" << m_protocolHeaderID;
            }

          public:
            ~CBaseSMChannelImpl<T>()
            {
                LOG(MiscCommon::info) << "SM: channel destructor is called. MQ: " << getName();
                stop();
            }

            static connectionPtr_t makeNew(boost::asio::io_context& _service,
                                           const std::string& _inputName,
                                           const std::string& _outputName,
                                           uint64_t _ProtocolHeaderID,
                                           EMQOpenType _inputOpenType = EMQOpenType::OpenOrCreate,
                                           EMQOpenType _outputOpenType = EMQOpenType::OpenOrCreate)
            {
                connectionPtr_t newObject(
                    new T(_service, _inputName, _outputName, _ProtocolHeaderID, _inputOpenType, _outputOpenType));
                return newObject;
            }

            static connectionPtr_t makeNew(boost::asio::io_context& _service,
                                           const std::vector<std::string>& _inputNames,
                                           const std::string& _outputName,
                                           uint64_t _ProtocolHeaderID,
                                           EMQOpenType _inputOpenType = EMQOpenType::OpenOrCreate,
                                           EMQOpenType _outputOpenType = EMQOpenType::OpenOrCreate)
            {
                connectionPtr_t newObject(
                    new T(_service, _inputNames, _outputName, _ProtocolHeaderID, _inputOpenType, _outputOpenType));
                return newObject;
            }

          private:
            void createMessageQueue()
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportIn);
                    for (auto& info : m_transportIn)
                    {
                        LOG(MiscCommon::info) << "SM: Initializing input message queue: " << info.m_name;
                        info.m_mq.reset();
                        info.m_mq = createMessageQueue(info.m_name.c_str(), info.m_openType);
                    }
                }

                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                    for (auto& v : m_outputBuffers)
                    {
                        SMessageQueueInfo& info = v.second->m_info;
                        LOG(MiscCommon::info) << "SM: Initializing output message queue: " << info.m_name;
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
                // TODO: Because of performance problems we had to reduce the size of the message from 65K to 1K.
                // TODO: Need to implement an algorithm to break protocol messages on smaller chunks if they are bigger
                // than nmaxMessageSize
                static const unsigned int maxMessageSize = 1024;

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
                        default:
                            LOG(MiscCommon::error)
                                << "Can't initialize shared memory transport with name " << _name << ": "
                                << "Unknown EMQOpenType given: " << static_cast<int>(_openType);
                            return nullptr;
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
            uint64_t getProtocolHeaderID() const
            {
                return m_protocolHeaderID;
            }

            void addOutput(uint64_t _outputID,
                           const std::string& _name,
                           EMQOpenType _openType = EMQOpenType::OpenOrCreate)
            {
                // We use EMQOpenType::OpenOrCreate in order to prevent race condition when the leader is already
                // selected, but the shared memory has not been created yet. If the shared memory was not created by the
                // lobby leader it will be automatically created by the lobby member.

                if (_outputID < 1)
                {
                    std::stringstream ss;
                    ss << "Can't add output " << _name
                       << ". Output ID must be greater than 0. Current value: " << _outputID;
                    throw std::runtime_error(ss.str());
                }

                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                    auto it = m_outputBuffers.find(_outputID);
                    if (it != m_outputBuffers.end())
                    {
                        std::stringstream ss;
                        ss << "Can't add output " << _name << ". Output with ID " << _outputID << " already exists.";
                        throw std::runtime_error(ss.str());
                    }
                }

                auto buffer = std::make_shared<SMessageOutputBuffer>();
                buffer->m_info.m_name = _name;
                buffer->m_info.m_openType = _openType;
                buffer->m_info.m_mq = createMessageQueue(_name, _openType);

                if (buffer->m_info.m_mq != nullptr)
                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                    auto result = m_outputBuffers.emplace(_outputID, buffer);
                    if (!result.second)
                    {
                        std::stringstream ss;
                        ss << "Failed to add shared memory channel output with ID: " << _outputID << " name: " << _name;
                        throw std::runtime_error(ss.str());
                    }
                    else
                    {
                        LOG(MiscCommon::info)
                            << "Added shared memory channel output with ID: " << _outputID << " name: " << _name;
                    }
                }
                else
                {
                    std::stringstream ss;
                    ss << "Can't add shared memory channel output with ID: " << _outputID << " name: " << _name;
                    throw std::runtime_error(ss.str());
                }
            }

            bool started() const
            {
                return m_started;
            }

            void start()
            {
                // Check that all message queues were succesfully created
                bool queuesCreated(true);
                for (const auto& v : m_transportIn)
                {
                    if (v.m_mq == nullptr)
                    {
                        queuesCreated = false;
                        break;
                    }
                }

                if (queuesCreated)
                {
                    for (const auto& v : m_outputBuffers)
                    {
                        if (v.second->m_info.m_mq == nullptr)
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
                m_isShuttingDown = false;

                auto self(this->shared_from_this());
                for (const auto& v : m_transportIn)
                {
                    m_ioContext.post([this, self, &v] {
                        try
                        {
                            readMessage(v);
                        }
                        catch (std::exception& ex)
                        {
                            LOG(MiscCommon::error) << "BaseSMChannelImpl can't read message: " << ex.what();
                        }
                    });
                }

                SSenderInfo sender;
                sender.m_ID = m_protocolHeaderID;
            }

            void stop()
            {
                LOG(MiscCommon::info) << "SM: channel STOP is called. MQ: " << getName();
                if (!m_started)
                    return;

                m_started = false;
                m_isShuttingDown = true;
            }

            void removeMessageQueue()
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportIn);
                    for (const auto& v : m_transportIn)
                    {
                        const bool status = boost::interprocess::message_queue::remove(v.m_name.c_str());
                        LOG(MiscCommon::info) << "Message queue " << v.m_name << " remove status: " << status;
                    }
                }
                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                    for (const auto& v : m_outputBuffers)
                    {
                        const SMessageQueueInfo& info = v.second->m_info;
                        const bool status = boost::interprocess::message_queue::remove(info.m_name.c_str());
                        LOG(MiscCommon::info) << "Message queue " << info.m_name << " remove status: " << status;
                    }
                }
            }

            void pushMsg(CProtocolMessage::protocolMessagePtr_t _msg, ECmdType _cmd, uint64_t _outputID = 0)
            {
                if (!m_started)
                {
                    LOG(MiscCommon::error) << "Skip pushing message. The channel was not started.";
                    return;
                }

                if (m_isShuttingDown)
                {
                    LOG(MiscCommon::warning) << "Skip pushing message. The channel is shutting down.";
                    return;
                }

                try
                {
                    // Get corresponding buffer
                    const typename SMessageOutputBuffer::Ptr_t& buffer = getOutputBuffer(_outputID);

                    std::lock_guard<std::mutex> lock(buffer->m_mutexWriteBuffer);

                    // Need to drain the queue, i.e. skip messages
                    if (buffer->m_drainWriteQueue)
                    {
                        buffer->m_writeQueue.clear();
                        return;
                    }

                    // add the current message to the queue
                    if (cmdUNKNOWN != _cmd)
                        buffer->m_writeQueue.push_back(SProtocolMessageInfo(_outputID, _msg));

                    LOG(MiscCommon::debug)
                        << getName()
                        << ": BaseSMChannelImpl pushMsg: WriteQueue size = " << buffer->m_writeQueue.size();

                    // process standard async writing
                    auto self(this->shared_from_this());
                    m_ioContext.post([this, self, &buffer] {
                        try
                        {
                            writeMessage(buffer);
                        }
                        catch (std::exception& ex)
                        {
                            LOG(MiscCommon::error) << "BaseSMChannelImpl can't write message: " << ex.what();
                        }
                    });
                }
                catch (std::exception& ex)
                {
                    LOG(MiscCommon::error) << getName() << ":  BaseSMChannelImpl can't push message: " << ex.what();
                }
            }

            template <ECmdType _cmd, class A>
            void pushMsg(const A& _attachment, uint64_t _protocolHeaderID = 0, uint64_t _outputID = 0)
            {
                try
                {
                    uint64_t headerID = adjustProtocolHeaderID(_protocolHeaderID);
                    CProtocolMessage::protocolMessagePtr_t msg =
                        SCommandAttachmentImpl<_cmd>::encode(_attachment, headerID);
                    pushMsg(msg, _cmd, _outputID);
                }
                catch (std::exception& ex)
                {
                    LOG(MiscCommon::error) << "BaseSMChannelImpl can't push message: " << ex.what();
                }
            }

            template <ECmdType _cmd>
            void pushMsg(uint64_t _protocolHeaderID = 0, uint64_t _outputID = 0)
            {
                SEmptyCmd cmd;
                pushMsg<_cmd>(cmd, _protocolHeaderID, _outputID);
            }

            void drainWriteQueue(bool _newVal, uint64_t _outputID)
            {
                const typename SMessageOutputBuffer::Ptr_t& buffer = getOutputBuffer(_outputID);
                buffer->m_drainWriteQueue = _newVal;
            }

          private:
            const typename SMessageOutputBuffer::Ptr_t& getOutputBuffer(uint64_t _outputID)
            {
                std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                auto it = m_outputBuffers.find(_outputID);
                if (it != m_outputBuffers.end())
                    return it->second;

                throw std::runtime_error("Can't find corresponding output buffer: " + std::to_string(_outputID));
            }

            uint64_t adjustProtocolHeaderID(uint64_t _protocolHeaderID) const
            {
                return (_protocolHeaderID == 0) ? m_protocolHeaderID : _protocolHeaderID;
            }

            void readMessage(const SMessageQueueInfo& _info)
            {
                try
                {
                    CProtocolMessage::protocolMessagePtr_t currentMsg = std::make_shared<CProtocolMessage>();

                    unsigned int priority;
                    boost::interprocess::message_queue::size_type receivedSize;

                    // We need to allocate the memory of the size equal to the maximum size of the message
                    currentMsg->resize(_info.m_mq->get_max_msg_size());

                    namespace pt = boost::posix_time;
                    while (!_info.m_mq->timed_receive(
                        currentMsg->data(),
                        _info.m_mq->get_max_msg_size(),
                        receivedSize,
                        priority,
                        pt::ptime(pt::microsec_clock::universal_time()) + pt::milliseconds(500)))
                    {
                        if (m_isShuttingDown)
                        {
                            LOG(MiscCommon::info) << _info.m_name << ": stopping read operation due to shutdown";
                            return;
                        }
                    }

                    if (receivedSize < CProtocolMessage::header_length)
                    {
                        LOG(MiscCommon::warning)
                            << _info.m_name << ": Received message: " << receivedSize << " bytes, expected at least"
                            << CProtocolMessage::header_length << " bytes";
                    }
                    else
                    {
                        // Resize message data to the actually received bytes
                        currentMsg->resize(receivedSize);
                        if (currentMsg->decode_header())
                        {
                            // If the header is ok, process the body of the message
                            processBody(receivedSize - CProtocolMessage::header_length, _info, currentMsg);
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

            void processBody(boost::interprocess::message_queue::size_type _bodySize,
                             const SMessageQueueInfo& _info,
                             const CProtocolMessage::protocolMessagePtr_t& _currentMsg)
            {
                if (_bodySize != _currentMsg->body_length())
                {
                    LOG(MiscCommon::error) << _info.m_name << ": Received message BODY: " << _bodySize
                                           << " bytes, expected " << _currentMsg->body_length();
                }
                else
                {
                    if (_currentMsg->body_length() == 0)
                    {
                        LOG(MiscCommon::debug)
                            << _info.m_name << ": Received message BODY no attachment: " << _currentMsg->toString();
                    }
                    else
                    {
                        LOG(MiscCommon::debug) << _info.m_name << ": Received message BODY (" << _bodySize
                                               << " bytes): " << _currentMsg->toString();
                    }

                    // process received message
                    T* pThis = static_cast<T*>(this);
                    pThis->processMessage(_currentMsg);

                    auto self(this->shared_from_this());
                    m_ioContext.post([this, self, &_info] {
                        try
                        {
                            readMessage(_info);
                        }
                        catch (std::exception& ex)
                        {
                            LOG(MiscCommon::error) << "BaseSMChannelImpl can't read message: " << ex.what();
                        }
                    });
                }
            }

            void writeMessage(const typename SMessageOutputBuffer::Ptr_t& _buffer)
            {
                if (_buffer == nullptr)
                    throw std::runtime_error("Can't find corresponding output buffer");

                {
                    std::lock_guard<std::mutex> lockWriteBuffer(_buffer->m_mutexWriteBuffer);
                    if (!_buffer->m_writeBufferQueue.empty())
                        return; // A write is in progress, don't start anything

                    if (_buffer->m_writeQueue.empty())
                        return; // There is nothing to send.

                    _buffer->m_writeBufferQueue.assign(_buffer->m_writeQueue.begin(), _buffer->m_writeQueue.end());
                    _buffer->m_writeQueue.clear();
                }

                try
                {
                    for (auto& msg : _buffer->m_writeBufferQueue)
                    {
                        if (_buffer->m_info.m_mq != nullptr)
                        {
                            namespace pt = boost::posix_time;
                            while (!_buffer->m_info.m_mq->timed_send(
                                msg.m_msg->data(),
                                msg.m_msg->length(),
                                0,
                                pt::ptime(pt::microsec_clock::universal_time()) + pt::milliseconds(500)))
                            {
                                if (m_isShuttingDown)
                                {
                                    LOG(MiscCommon::info)
                                        << _buffer->m_info.m_name << ": stopping write operation due to shutdown";
                                    return;
                                }

                                // If the other end is disconnected or the queue is full, we
                                // will block the thread by infinitely retrying to send.
                                // For such cases there is a drain command. The connection manager can initiate the
                                // drain, when needed. For example in case when a user task disconnects from the
                                // Intercom channel a drain will be initiated until we receive a new task assignment.
                                if (_buffer->m_drainWriteQueue)
                                {
                                    LOG(MiscCommon::warning)
                                        << _buffer->m_info.m_name
                                        << ": Draining write queue, while there is a message pending: "
                                        << g_cmdToString[msg.m_msg->header().m_cmd];
                                    break;
                                }
                            }
                        }
                        else
                        {
                            LOG(MiscCommon::error)
                                << _buffer->m_info.m_name << ": Can't find output transport with output ID "
                                << msg.m_outputID
                                << ". Write message failed. Command: " << g_cmdToString[msg.m_msg->header().m_cmd];
                        }
                    }
                }
                catch (boost::interprocess::interprocess_exception& ex)
                {
                    LOG(MiscCommon::error)
                        << _buffer->m_info.m_name << ": BaseSMChannelImpl: error sending message: " << ex.what();
                }

                // Lock the modification of the container
                {
                    std::lock_guard<std::mutex> lock(_buffer->m_mutexWriteBuffer);
                    _buffer->m_writeBufferQueue.clear();
                }
                // We might need to send more messages
                writeMessage(_buffer);
            }

            const std::string& getName() const
            {
                return m_transportIn.front().m_name;
            }

          protected:
            std::atomic<bool> m_isShuttingDown;
            std::atomic<bool> m_started; ///< True if we were able to start the channel, False otherwise
            uint64_t m_protocolHeaderID;

          private:
            boost::asio::io_context& m_ioContext; ///< IO service that is used as a thread pool

            typename SMessageQueueInfo::Container_t
                m_transportIn; ///< Vector of input message queues, i.e. we read from this queues
            typename SMessageOutputBuffer::Container_t m_outputBuffers;
            std::mutex m_mutexTransportIn;  ///< Mutex for transport input map
            std::mutex m_mutexTransportOut; ///< Mutex for transport output map
        };
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__BaseSMChannelImpl__) */
