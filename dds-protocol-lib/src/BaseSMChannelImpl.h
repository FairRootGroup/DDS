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

#define SM_MESSAGE_HANDLER_DISPATCH(msg)                                                                         \
    case msg:                                                                                                    \
    {                                                                                                            \
        processed = false;                                                                                       \
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
                std::string m_name;     ///< Message queue name
                EMQOpenType m_openType; ///< Message queue open type
                messageQueuePtr_t m_mq; ///< Message queue
            };

            typedef std::map<uint64_t, SMessageQueueInfo> messageQueueMap_t;
            typedef std::vector<SMessageQueueInfo> messageQueueVector_t;

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
                SMessageQueueInfo outInfo;
                outInfo.m_name = _outputName;
                outInfo.m_openType = _outputOpenType;
                m_transportOut.emplace(0, outInfo);

                createMessageQueue();

                LOG(MiscCommon::info) << "SM: New channel: inputName=" << m_transportIn.front().m_name
                                      << " outputName=" << m_transportOut[0].m_name
                                      << " protocolHeaderID=" << m_protocolHeaderID;
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
                for (auto& info : m_transportIn)
                {
                    LOG(MiscCommon::info) << "SM: Initializing input message queue: " << info.m_name;
                    info.m_mq.reset();
                    info.m_mq = createMessageQueue(info.m_name.c_str(), info.m_openType);
                }

                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                    for (auto& v : m_transportOut)
                    {
                        SMessageQueueInfo& info = v.second;
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
                    auto it = m_transportOut.find(_outputID);
                    if (it != m_transportOut.end())
                    {
                        std::stringstream ss;
                        ss << "Can't add output " << _name << ". Output with ID " << _outputID << " already exists.";
                        throw std::runtime_error(ss.str());
                    }
                }

                SMessageQueueInfo info;
                info.m_name = _name;
                info.m_openType = _openType;
                info.m_mq = createMessageQueue(_name, _openType);

                if (info.m_mq != nullptr)
                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                    auto result = m_transportOut.emplace(_outputID, info);
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
                dispatchHandlers(EChannelEvents::OnSMStart, sender);
            }

            void stop()
            {
                LOG(MiscCommon::info) << "SM: channel STOP is called. MQ: " << getName();
                if (!m_started)
                    return;

                m_started = false;
                sendYourselfShutdown();
            }

            void removeMessageQueue()
            {
                for (const auto& v : m_transportIn)
                {
                    const bool status = boost::interprocess::message_queue::remove(v.m_name.c_str());
                    LOG(MiscCommon::info) << "Message queue " << v.m_name << " remove status: " << status;
                }
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

            void pushMsg(CProtocolMessage::protocolMessagePtr_t _msg, ECmdType _cmd, uint64_t _outputID = 0)
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
                        m_writeQueue.push_back(SProtocolMessageInfo(_outputID, _msg));

                    LOG(MiscCommon::debug)
                        << getName() << ": BaseSMChannelImpl pushMsg: WriteQueue size = " << m_writeQueue.size();
                }
                catch (std::exception& ex)
                {
                    LOG(MiscCommon::error) << getName() << ":  BaseSMChannelImpl can't push message: " << ex.what();
                }

                // process standard async writing
                auto self(this->shared_from_this());
                m_ioContext.post([this, self] {
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

            void syncSendShutdownAll()
            {
                // Send cmdSHUTDOWN to all connected outputs except yourself
                {
                    std::lock_guard<std::mutex> lock(m_mutexTransportOut);
                    for (auto it : m_transportOut)
                    {
                        if (it.first == 0 || it.first == m_protocolHeaderID)
                            continue;
                        SEmptyCmd cmd;
                        CProtocolMessage::protocolMessagePtr_t msg =
                            SCommandAttachmentImpl<cmdSHUTDOWN>::encode(cmd, m_protocolHeaderID);
                        messageQueuePtr_t mq = it.second.m_mq;
                        // TODO: FIXME: Revise the code, use timed_send
                        mq->send(msg->data(), msg->length(), 1);
                    }
                }
            }

          private:
            uint64_t adjustProtocolHeaderID(uint64_t _protocolHeaderID) const
            {
                return (_protocolHeaderID == 0) ? m_protocolHeaderID : _protocolHeaderID;
            }

            void sendYourselfShutdown()
            {
                LOG(MiscCommon::debug) << m_transportIn.front().m_name << ": Sends itself a SHUTDOWN msg";
                // Send cmdSHUTDOWN with higher priority in order to stop read operation.
                SEmptyCmd cmd;
                CProtocolMessage::protocolMessagePtr_t msg =
                    SCommandAttachmentImpl<cmdSHUTDOWN>::encode(cmd, m_protocolHeaderID);
                // m_transportIn.m_mq->send(msg->data(), msg->length(), 1);
                boost::system_time const timeout = boost::get_system_time() + boost::posix_time::milliseconds(500);
                while (!m_transportIn.front().m_mq->timed_send(msg->data(), msg->length(), 0, timeout))
                {
                    if (m_isShuttingDown)
                    {
                        LOG(MiscCommon::debug) << m_transportIn.front().m_name
                                               << ": stopping send yourself shutdown while already shutting down";
                        return;
                    }
                }
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
                    _info.m_mq->receive(currentMsg->data(), _info.m_mq->get_max_msg_size(), receivedSize, priority);

                    if (receivedSize < CProtocolMessage::header_length)
                    {
                        LOG(MiscCommon::debug)
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

                    ECmdType currentCmd = static_cast<ECmdType>(_currentMsg->header().m_cmd);

                    // Do not start readMessage if cmdSHUTDOWN was sent
                    if (currentCmd != cmdSHUTDOWN)
                    {
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
                    else
                    {
                        m_isShuttingDown = true;
                        LOG(MiscCommon::debug) << _info.m_name << ": Stopping readMessage thread...";
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
                            auto it = m_transportOut.find(msg.m_outputID);
                            exists = (it != m_transportOut.end());
                            mq = it->second.m_mq;
                        }

                        if (exists && mq != nullptr)
                        {
                            boost::system_time const timeout =
                                boost::get_system_time() + boost::posix_time::milliseconds(500);
                            while (!mq->timed_send(msg.m_msg->data(), msg.m_msg->length(), 0, timeout))
                            {
                                if (m_isShuttingDown)
                                {
                                    LOG(MiscCommon::debug) << getName() << ": stopping write operation due to shutdown";
                                    return;
                                }
                            }
                        }
                        else
                        {
                            LOG(MiscCommon::error)
                                << "Can't find output transport with output ID " << msg.m_outputID
                                << ". Write message failed. Command: " << g_cmdToString[msg.m_msg->header().m_cmd];
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

            messageQueueVector_t m_transportIn; ///< Vector of input message queues, i.e. we read from this queues
            messageQueueMap_t m_transportOut;   ///< Map of output message queues, i.e. we write to this queues
            std::mutex m_mutexTransportOut;     ///< Mutex for transport output map

            protocolMessagePtrQueue_t m_writeQueue; ///< Cache for the messages that we want to send
            std::mutex m_mutexWriteBuffer;
            protocolMessagePtrQueue_t m_writeBufferQueue;
        };
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__BaseSMChannelImpl__) */
