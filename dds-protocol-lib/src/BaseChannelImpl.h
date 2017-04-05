// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BaseChannelImpl__
#define __DDS__BaseChannelImpl__
// STD
#include <chrono>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
// BOOST
#include <boost/noncopyable.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <boost/asio.hpp>
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#pragma clang diagnostic pop
// DDS
#include "ChannelEventsImpl.h"
#include "ChannelMessageHandlersImpl.h"
#include "CommandAttachmentImpl.h"
#include "Logger.h"
#include "MonitoringThread.h"
#include "StatImpl.h"

namespace dds
{
    namespace protocol_api
    {
        template <class T>
        class CClientChannelImpl; // needed for friend class
        template <class T>
        class CServerChannelImpl; // needed for friend class
    }
}

#define BEGIN_MSG_MAP(theClass)                                                                                        \
  public:                                                                                                              \
    friend protocol_api::CBaseChannelImpl<theClass>;                                                                   \
    friend protocol_api::CClientChannelImpl<theClass>;                                                                 \
    friend protocol_api::CServerChannelImpl<theClass>;                                                                 \
    void processMessage(protocol_api::CProtocolMessage::protocolMessagePtr_t _currentMsg)                              \
    {                                                                                                                  \
        using namespace dds;                                                                                           \
        using namespace dds::protocol_api;                                                                             \
        CMonitoringThread::instance().updateIdle();                                                                    \
        bool processed = true;                                                                                         \
        ECmdType currentCmd = static_cast<ECmdType>(_currentMsg->header().m_cmd);                                      \
                                                                                                                       \
        try                                                                                                            \
        {                                                                                                              \
            switch (currentCmd)                                                                                        \
            {                                                                                                          \
                case cmdBINARY_ATTACHMENT:                                                                             \
                {                                                                                                      \
                    typedef typename SCommandAttachmentImpl<cmdBINARY_ATTACHMENT>::ptr_t attahcmentPtr_t;              \
                    attahcmentPtr_t attachmentPtr = SCommandAttachmentImpl<cmdBINARY_ATTACHMENT>::decode(_currentMsg); \
                    processBinaryAttachmentCmd(attachmentPtr);                                                         \
                    return;                                                                                            \
                }                                                                                                      \
                case cmdBINARY_ATTACHMENT_START:                                                                       \
                {                                                                                                      \
                    typedef typename SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_START>::ptr_t attahcmentPtr_t;        \
                    attahcmentPtr_t attachmentPtr =                                                                    \
                        SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_START>::decode(_currentMsg);                       \
                    processBinaryAttachmentStartCmd(attachmentPtr);                                                    \
                    return;                                                                                            \
                }                                                                                                      \
                case cmdHANDSHAKE:                                                                                     \
                {                                                                                                      \
                    SCommandAttachmentImpl<cmdHANDSHAKE>::ptr_t attachmentPtr =                                        \
                        SCommandAttachmentImpl<cmdHANDSHAKE>::decode(_currentMsg);                                     \
                    dispatchHandlers(currentCmd, attachmentPtr, this);                                                 \
                    return;                                                                                            \
                }                                                                                                      \
                case cmdREPLY_HANDSHAKE_OK:                                                                            \
                {                                                                                                      \
                    SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t attachmentPtr =                               \
                        SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::decode(_currentMsg);                            \
                    dispatchHandlers(currentCmd, attachmentPtr, this);                                                 \
                    return;                                                                                            \
                }                                                                                                      \
                case cmdREPLY_HANDSHAKE_ERR:                                                                           \
                {                                                                                                      \
                    SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_ERR>::ptr_t attachmentPtr =                              \
                        SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_ERR>::decode(_currentMsg);                           \
                    dispatchHandlers(currentCmd, attachmentPtr, this);                                                 \
                    return;                                                                                            \
                }

#define MESSAGE_HANDLER(msg, func)                                                                                 \
    case msg:                                                                                                      \
    {                                                                                                              \
        typedef typename SCommandAttachmentImpl<msg>::ptr_t attahcmentPtr_t;                                       \
        attahcmentPtr_t attachmentPtr = SCommandAttachmentImpl<msg>::decode(_currentMsg);                          \
        LOG(MiscCommon::debug) << "Processing " << g_cmdToString[msg] << " received from " << remoteEndIDString(); \
        processed = func(attachmentPtr);                                                                           \
        if (!processed)                                                                                            \
        {                                                                                                          \
            if (!handlerExists(msg))                                                                               \
            {                                                                                                      \
                LOG(MiscCommon::error) << "The received message was not processed and has no registered handler: " \
                                       << _currentMsg->toString();                                                 \
            }                                                                                                      \
            else                                                                                                   \
            {                                                                                                      \
                dispatchHandlers(msg, attachmentPtr, this);                                                        \
            }                                                                                                      \
        }                                                                                                          \
        break;                                                                                                     \
    }

#define END_MSG_MAP()                                                                                         \
    default:                                                                                                  \
        LOG(MiscCommon::error) << "The received message doesn't have a handler: " << _currentMsg->toString(); \
        }                                                                                                     \
        }                                                                                                     \
        catch (std::exception & _e)                                                                           \
        {                                                                                                     \
            LOG(MiscCommon::error) << "Channel processMessage: " << _e.what();                                \
        }                                                                                                     \
        }

#define REGISTER_DEFAULT_REMOTE_ID_STRING \
    std::string _remoteEndIDString()      \
    {                                     \
        return "DDS Server";              \
    }

namespace dds
{
    namespace protocol_api
    {
        // Channel types
        enum EChannelType
        {
            UNKNOWN = 0,
            AGENT,
            UI,
            API_GUARD
        };
        typedef std::vector<EChannelType> channelTypeVector_t;
        const std::array<std::string, 5> gChannelTypeName{
            { "unknown", "agent", "ui", "key_value_guard", "custom_command_guard" }
        };

        struct SBinaryAttachmentInfo
        {
            SBinaryAttachmentInfo()
                : m_bytesReceived(0)
                , m_fileCrc32(0)
                , m_srcCommand(0)
                , m_fileSize(0)
                , m_startTime()
            {
            }

            MiscCommon::BYTEVector_t m_data;
            uint32_t m_bytesReceived;
            std::string m_fileName;
            uint32_t m_fileCrc32;
            uint16_t m_srcCommand;
            uint32_t m_fileSize;
            std::mutex m_mutex;
            std::chrono::steady_clock::time_point m_startTime;
        };

        typedef std::shared_ptr<SBinaryAttachmentInfo> binaryAttachmentInfoPtr_t;

        template <class T>
        class CBaseChannelImpl : public boost::noncopyable,
                                 public CChannelEventsImpl<T>,
                                 public CChannelMessageHandlersImpl,
                                 public std::enable_shared_from_this<T>,
                                 public CStatImpl
        {
            typedef std::function<void(T*)> handlerDisconnectEventFunction_t;
            typedef std::deque<CProtocolMessage::protocolMessagePtr_t> protocolMessagePtrQueue_t;
            typedef std::vector<boost::asio::mutable_buffer> protocolMessageBuffer_t;
            typedef std::shared_ptr<boost::asio::deadline_timer> deadlineTimerPtr_t;

          public:
            typedef std::shared_ptr<T> connectionPtr_t;
            typedef std::weak_ptr<T> weakConnectionPtr_t;
            typedef std::vector<connectionPtr_t> connectionPtrVector_t;
            typedef std::vector<weakConnectionPtr_t> weakConnectionPtrVector_t;

          protected:
            CBaseChannelImpl<T>(boost::asio::io_service& _service)
                : CChannelEventsImpl<T>()
                , CChannelMessageHandlersImpl()
                , CStatImpl(_service)
                , m_isHandshakeOK(false)
                , m_channelType(EChannelType::UNKNOWN)
                , m_io_service(_service)
                , m_socket(_service)
                , m_started(false)
                , m_currentMsg(std::make_shared<CProtocolMessage>())
                , m_binaryAttachmentMap()
                , m_binaryAttachmentMutex()
                , m_deadlineTimer(
                      std::make_shared<boost::asio::deadline_timer>(_service, boost::posix_time::milliseconds(1000)))
                , m_isShuttingDown(false)
            {
            }

          public:
            ~CBaseChannelImpl<T>()
            {
                LOG(MiscCommon::info) << "Channel " << gChannelTypeName[m_channelType] << " destructor is called";
                stop();
            }

            static connectionPtr_t makeNew(boost::asio::io_service& _service)
            {
                connectionPtr_t newObject(new T(_service));
                return newObject;
            }

          public:
            bool isHanshakeOK() const
            {
                return m_isHandshakeOK;
            }

            EChannelType getChannelType() const
            {
                return m_channelType;
            }

            void setChannelType(EChannelType _channelType)
            {
                m_channelType = _channelType;
            }

          public:
            void start()
            {
                if (m_started)
                    return;

                m_started = true;
                // Prevent Asio TCP socket to be inherited by child when using fork/exec
                ::fcntl(m_socket.native_handle(), F_SETFD, FD_CLOEXEC);
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

            template <ECmdType _cmd>
            void dequeueMsg()
            {
                std::lock_guard<std::mutex> lock(m_mutexWriteBuffer);
                m_writeQueue.erase(std::remove_if(std::begin(m_writeQueue),
                                                  std::end(m_writeQueue),
                                                  [](const CProtocolMessage::protocolMessagePtr_t& _msg) {
                                                      return (_msg->header().m_cmd == _cmd);
                                                  }),
                                   std::end(m_writeQueue));
            }

            template <ECmdType _cmd, class A>
            void accumulativePushMsg(const A& _attachment)
            {
                static const size_t maxAccumulativeWriteQueueSize = 10000;
                try
                {
                    CProtocolMessage::protocolMessagePtr_t msg = SCommandAttachmentImpl<_cmd>::encode(_attachment);

                    bool copyMessages = false;
                    {
                        std::lock_guard<std::mutex> lock(m_mutexWriteBuffer);

                        m_deadlineTimer->cancel();

                        if (cmdUNKNOWN != _cmd)
                            m_accumulativeWriteQueue.push_back(msg);

                        copyMessages = m_accumulativeWriteQueue.size() > maxAccumulativeWriteQueueSize;
                        if (copyMessages)
                        {
                            LOG(MiscCommon::debug)
                                << "copy accumulated queue to write queue "
                                   "m_accumulativeWriteQueue.size="
                                << m_accumulativeWriteQueue.size() << " m_writeQueue.size=" << m_writeQueue.size();

                            // copy queue to main queue
                            std::copy(m_accumulativeWriteQueue.begin(),
                                      m_accumulativeWriteQueue.end(),
                                      back_inserter((m_isHandshakeOK) ? m_writeQueue : m_writeQueueBeforeHandShake));
                            m_accumulativeWriteQueue.clear();
                        }

                        auto self(this->shared_from_this());
                        m_deadlineTimer->async_wait([this, self](const boost::system::error_code& error) {
                            if (!error)
                            {
                                bool copyMessages = false;
                                {
                                    std::lock_guard<std::mutex> lock(m_mutexWriteBuffer);
                                    copyMessages = !m_accumulativeWriteQueue.empty();
                                    // copy queue to main queue
                                    if (copyMessages)
                                    {
                                        LOG(MiscCommon::debug)
                                            << "deadline_timer called: copy accumulated queue to write queue "
                                               "m_accumulativeWriteQueue.size="
                                            << m_accumulativeWriteQueue.size()
                                            << " m_writeQueue.size=" << m_writeQueue.size();
                                        std::copy(m_accumulativeWriteQueue.begin(),
                                                  m_accumulativeWriteQueue.end(),
                                                  back_inserter((m_isHandshakeOK) ? m_writeQueue
                                                                                  : m_writeQueueBeforeHandShake));
                                        m_accumulativeWriteQueue.clear();
                                    }
                                }
                                if (copyMessages)
                                    pushMsg<cmdUNKNOWN>();
                            }
                        });

                        LOG(MiscCommon::debug) << "accumulativePushMsg: WriteQueue size = " << m_writeQueue.size()
                                               << " WriteQueueBeforeHandShake = " << m_writeQueueBeforeHandShake.size()
                                               << " accumulativeWriteQueue size = " << m_accumulativeWriteQueue.size()
                                               << " attachment = " << _attachment;
                    }
                    if (copyMessages)
                        pushMsg<cmdUNKNOWN>();
                }
                catch (std::exception& ex)
                {
                    LOG(MiscCommon::error) << "BaseChannelImpl can't push accumulative message: " << ex.what();
                }
            }

            template <ECmdType _cmd>
            void accumulativePushMsg()
            {
                SEmptyCmd cmd;
                accumulativePushMsg<_cmd>(cmd);
            }

            template <ECmdType _cmd, class A>
            void pushMsg(const A& _attachment)
            {
                try
                {
                    CProtocolMessage::protocolMessagePtr_t msg = SCommandAttachmentImpl<_cmd>::encode(_attachment);

                    std::lock_guard<std::mutex> lock(m_mutexWriteBuffer);
                    if (!m_isHandshakeOK)
                    {
                        if (isCmdAllowedWithoutHandshake(_cmd))
                            m_writeQueue.push_back(msg);
                        else
                            m_writeQueueBeforeHandShake.push_back(msg);
                    }
                    else
                    {
                        // copy the buffered queue, which has been collected before hand-shake
                        if (!m_writeQueueBeforeHandShake.empty())
                        {
                            std::copy(m_writeQueueBeforeHandShake.begin(),
                                      m_writeQueueBeforeHandShake.end(),
                                      back_inserter(m_writeQueue));
                            m_writeQueueBeforeHandShake.clear();
                        }

                        // add the current message to the queue
                        if (cmdUNKNOWN != _cmd)
                            m_writeQueue.push_back(msg);
                    }

                    LOG(MiscCommon::debug) << "pushMsg: WriteQueue size = " << m_writeQueue.size()
                                           << " WriteQueueBeforeHandShake = " << m_writeQueueBeforeHandShake.size();
                }
                catch (std::exception& ex)
                {
                    LOG(MiscCommon::error) << "BaseChannelImpl can't push message: " << ex.what();
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
                        LOG(MiscCommon::error) << "BaseChannelImpl can't write message: " << ex.what();
                    }
                });
            }

            template <ECmdType _cmd>
            void pushMsg()
            {
                SEmptyCmd cmd;
                pushMsg<_cmd>(cmd);
            }

            template <ECmdType _cmd, class A>
            void sendYourself(const A& _attachment)
            {
                CProtocolMessage::protocolMessagePtr_t msg = SCommandAttachmentImpl<_cmd>::encode(_attachment);
                // process received message
                T* pThis = static_cast<T*>(this);
                pThis->processMessage(msg);
            }

            template <ECmdType _cmd>
            void sendYourself()
            {
                SEmptyCmd cmd;
                sendYourself<_cmd>(cmd);
            }

            void pushBinaryAttachmentCmd(const std::string& _srcFilePath,
                                         const std::string& _fileName,
                                         uint16_t _cmdSource)
            {
                MiscCommon::BYTEVector_t data;

                std::string srcFilePath(_srcFilePath);
                // Resolve environment variables
                MiscCommon::smart_path(&srcFilePath);

                std::ifstream f(srcFilePath);
                if (!f.is_open() || !f.good())
                {
                    throw std::runtime_error("Could not open the source file: " + srcFilePath);
                }
                f.seekg(0, std::ios::end);
                data.reserve(f.tellg());
                f.seekg(0, std::ios::beg);
                data.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

                pushBinaryAttachmentCmd(data, _fileName, _cmdSource);
            }

            void pushBinaryAttachmentCmd(const MiscCommon::BYTEVector_t& _data,
                                         const std::string& _fileName,
                                         uint16_t _cmdSource)
            {
                static const int maxCommandSize = 65536;
                int nofParts = (_data.size() % maxCommandSize == 0) ? (_data.size() / maxCommandSize)
                                                                    : (_data.size() / maxCommandSize + 1);
                boost::crc_32_type fileCrc32;
                fileCrc32.process_bytes(&_data[0], _data.size());

                boost::uuids::uuid fileId = boost::uuids::random_generator()();

                // Generate start message
                SBinaryAttachmentStartCmd start_cmd;
                start_cmd.m_fileId = fileId;
                start_cmd.m_srcCommand = _cmdSource;
                start_cmd.m_fileName = _fileName;
                start_cmd.m_fileSize = _data.size();
                start_cmd.m_fileCrc32 = fileCrc32.checksum();
                pushMsg<cmdBINARY_ATTACHMENT_START>(start_cmd);

                for (size_t i = 0; i < nofParts; ++i)
                {
                    SBinaryAttachmentCmd cmd;
                    cmd.m_fileId = fileId;

                    size_t offset = i * maxCommandSize;
                    size_t size = (i != (nofParts - 1)) ? maxCommandSize : (_data.size() - offset);

                    auto iter_begin = _data.begin() + offset;
                    auto iter_end = iter_begin + size;
                    std::copy(iter_begin, iter_end, std::back_inserter(cmd.m_data));

                    cmd.m_size = size;
                    cmd.m_offset = offset;

                    boost::crc_32_type crc32;
                    crc32.process_bytes(&(*iter_begin), size);

                    cmd.m_crc32 = crc32.checksum();

                    pushMsg<cmdBINARY_ATTACHMENT>(cmd);
                }
            }

            void processBinaryAttachmentStartCmd(SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_START>::ptr_t _attachment)
            {
                boost::uuids::uuid fileId = _attachment->m_fileId;

                {
                    // Lock with global map mutex
                    std::lock_guard<std::mutex> lock(m_binaryAttachmentMutex);

                    binaryAttachmentMap_t::iterator iter_info = m_binaryAttachmentMap.find(fileId);
                    bool exists = iter_info != m_binaryAttachmentMap.end();

                    if (!exists)
                    {
                        m_binaryAttachmentMap[fileId] = std::make_shared<SBinaryAttachmentInfo>();
                        iter_info = m_binaryAttachmentMap.find(fileId);
                        iter_info->second->m_startTime = std::chrono::steady_clock::now();
                        iter_info->second->m_fileName = _attachment->m_fileName;
                        iter_info->second->m_fileSize = _attachment->m_fileSize;
                        iter_info->second->m_fileCrc32 = _attachment->m_fileCrc32;
                        iter_info->second->m_srcCommand = _attachment->m_srcCommand;
                        iter_info->second->m_data.resize(_attachment->m_fileSize);
                    }
                }
            }

            void processBinaryAttachmentCmd(SCommandAttachmentImpl<cmdBINARY_ATTACHMENT>::ptr_t _attachment)
            {
                boost::uuids::uuid fileId = _attachment->m_fileId;
                binaryAttachmentInfoPtr_t info;
                binaryAttachmentMap_t::iterator iter_info;

                {
                    // Lock with global map mutex
                    std::lock_guard<std::mutex> lock(m_binaryAttachmentMutex);

                    iter_info = m_binaryAttachmentMap.find(fileId);
                    bool exists = iter_info != m_binaryAttachmentMap.end();

                    if (!exists)
                    {
                        LOG(MiscCommon::error)
                            << "Received binary attachment [" << fileId << "] which does not exist. Skip this message.";
                        return;
                    }
                    info = iter_info->second;
                }

                boost::crc_32_type crc32;
                crc32.process_bytes(&_attachment->m_data[0], _attachment->m_data.size());

                if (crc32.checksum() != _attachment->m_crc32)
                {
                    {
                        // Lock with global map mutex
                        std::lock_guard<std::mutex> lock(m_binaryAttachmentMutex);
                        // Remove info from map
                        m_binaryAttachmentMap.erase(iter_info);
                    }
                    std::stringstream ss;
                    ss << "Received binary attachment [" << fileId << "] has wrong CRC32 checksum: " << crc32.checksum()
                       << " instead of " << _attachment->m_crc32 << "offset=" << _attachment->m_offset
                       << " size=" << _attachment->m_size;
                    LOG(MiscCommon::error) << ss.str();
                    sendYourself<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, info->m_srcCommand));
                    return;
                }

                bool allBytesReceived = false;
                {
                    // Lock with local mutex for each file
                    std::lock_guard<std::mutex> lock(info->m_mutex);

                    info->m_bytesReceived += _attachment->m_size;

                    std::copy(_attachment->m_data.begin(),
                              _attachment->m_data.end(),
                              info->m_data.begin() + _attachment->m_offset);

                    allBytesReceived = info->m_bytesReceived == info->m_fileSize;
                    if (allBytesReceived)
                    {
                        // Check file CRC32
                        boost::crc_32_type crc32;
                        crc32.process_bytes(&info->m_data[0], info->m_data.size());

                        if (crc32.checksum() != info->m_fileCrc32)
                        {
                            {
                                // Lock with global map mutex
                                std::lock_guard<std::mutex> lock(m_binaryAttachmentMutex);
                                // Remove info from map
                                m_binaryAttachmentMap.erase(iter_info);
                            }
                            std::stringstream ss;
                            ss << "Received binary file [" << fileId
                               << "] has wrong CRC32 checksum: " << crc32.checksum() << " instead of "
                               << _attachment->m_crc32;
                            LOG(MiscCommon::error) << ss.str();
                            sendYourself<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, info->m_srcCommand));
                            return;
                        }

                        const std::string dir(user_defaults_api::CUserDefaults::getDDSPath());
                        const std::string fileName(dir + to_string(fileId));
                        std::ofstream f(fileName.c_str());
                        if (!f.is_open() || !f.good())
                        {
                            {
                                // Lock with global map mutex
                                std::lock_guard<std::mutex> lock(m_binaryAttachmentMutex);
                                // Remove info from map
                                m_binaryAttachmentMap.erase(iter_info);
                            }
                            std::stringstream ss;
                            ss << "Could not open file: " << fileName;
                            LOG(MiscCommon::error) << ss.str();
                            sendYourself<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, info->m_srcCommand));
                            return;
                        }

                        for (const auto& v : info->m_data)
                        {
                            f << v;
                        }
                        f.close();

                        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
                        std::chrono::microseconds downloadTime =
                            std::chrono::duration_cast<std::chrono::microseconds>(now - info->m_startTime);

                        // Send message to yourself
                        SBinaryAttachmentReceivedCmd reply_cmd;
                        reply_cmd.m_receivedFilePath = fileName;
                        reply_cmd.m_requestedFileName = info->m_fileName;
                        reply_cmd.m_srcCommand = info->m_srcCommand;
                        reply_cmd.m_downloadTime = downloadTime.count();
                        reply_cmd.m_receivedFileSize = info->m_fileSize;
                        sendYourself<cmdBINARY_ATTACHMENT_RECEIVED>(reply_cmd);
                    }
                }

                if (allBytesReceived)
                {
                    // Lock with global map mutex
                    std::lock_guard<std::mutex> lock(m_binaryAttachmentMutex);
                    // Remove info from map
                    m_binaryAttachmentMap.erase(iter_info);
                }
            }

            void registerDisconnectEventHandler(handlerDisconnectEventFunction_t _handler)
            {
                m_disconnectEventHandler = _handler;
            }

            bool started()
            {
                return m_started;
            }

            std::string remoteEndIDString()
            {
                // give a chance child to execute something
                try
                {
                    T* pThis = static_cast<T*>(this);
                    std::stringstream ss;
                    ss << pThis->_remoteEndIDString() << " [" << socket().remote_endpoint().address().to_string()
                       << "]";
                    return ss.str();
                }
                catch (...)
                {
                    return std::string();
                }
            }

          private:
            void readHeader()
            {
                auto self(this->shared_from_this());
                boost::asio::async_read(
                    m_socket,
                    boost::asio::buffer(m_currentMsg->data(), CProtocolMessage::header_length),
                    [this, self](boost::system::error_code ec, std::size_t length) {
                        if (!ec)
                        {
                            LOG(MiscCommon::debug) << "Received message HEADER from " << remoteEndIDString() << ": "
                                                   << length << " bytes, expected " << CProtocolMessage::header_length;
                        }
                        if (!ec && m_currentMsg->decode_header())
                        {
                            // If the header is ok, receive the body of the message
                            readBody();
                        }
                        else if ((boost::asio::error::eof == ec) || (boost::asio::error::connection_reset == ec))
                        {
                            LOG(MiscCommon::debug)
                                << "Disconnect is detected while on read msg header: " << ec.message();
                            onDissconnect();
                        }
                        else
                        {
                            if (m_started)
                                LOG(MiscCommon::error) << "Error reading message header: " << ec.message();
                            else
                                LOG(MiscCommon::info) << "The stop signal is received, aborting current operation and "
                                                         "closing the connection: "
                                                      << ec.message();

                            stop();
                        }
                    });
            }

            void readBody()
            {
                if (m_currentMsg->body_length() == 0)
                {
                    LOG(MiscCommon::debug) << "Received message BODY from " << remoteEndIDString()
                                           << ": no attachment: " << m_currentMsg->toString();
                    // process received message
                    T* pThis = static_cast<T*>(this);
                    pThis->processMessage(m_currentMsg);

                    // Log read statistics
                    this->logReadMessage(m_currentMsg);

                    // Read next message
                    m_currentMsg->clear();
                    readHeader();
                    return;
                }

                auto self(this->shared_from_this());
                boost::asio::async_read(
                    m_socket,
                    boost::asio::buffer(m_currentMsg->body(), m_currentMsg->body_length()),
                    [this, self](boost::system::error_code ec, std::size_t length) {
                        if (!ec)
                        {
                            LOG(MiscCommon::debug) << "Received message BODY from " << remoteEndIDString() << " ("
                                                   << length << " bytes): " << m_currentMsg->toString();

                            // process received message
                            T* pThis = static_cast<T*>(this);
                            pThis->processMessage(m_currentMsg);

                            // Log read statistics
                            this->logReadMessage(m_currentMsg);

                            // Read next message
                            m_currentMsg->clear();
                            readHeader();
                        }
                        else if ((boost::asio::error::eof == ec) || (boost::asio::error::connection_reset == ec))
                        {
                            LOG(MiscCommon::debug) << "Disconnect is detected while on read msg body: " << ec.message();
                            onDissconnect();
                        }
                        else
                        {
                            // don't show error if service is closed
                            if (m_started)
                                LOG(MiscCommon::error) << "Error reading message body: " << ec.message();
                            else
                                LOG(MiscCommon::info) << "The stop signal is received, aborting current operation and "
                                                         "closing the connection: "
                                                      << ec.message();
                            stop();
                        }
                    });
            }

          private:
            void writeMessage()
            {
                // To avoid sending of a bunch of small messages, we pack as many messages as possible into one write
                // request (GH-38).
                // Copy messages from the queue to send buffer (which should remain until the write handler is called)
                {
                    std::lock_guard<std::mutex> lockWriteBuffer(m_mutexWriteBuffer);
                    if (!m_writeBuffer.empty())
                        return; // a write is in progress, don't start anything

                    if (m_writeQueue.empty())
                        return; // There is nothing to send.

                    for (auto i : m_writeQueue)
                    {
                        LOG(MiscCommon::debug)
                            << "Sending to " << remoteEndIDString() << " a message: " << i->toString();
                        if (cmdSHUTDOWN == i->header().m_cmd)
                            m_isShuttingDown = true;
                        m_writeBuffer.push_back(boost::asio::buffer(i->data(), i->length()));
                        m_writeBufferQueue.push_back(i);
                    }
                    m_writeQueue.clear();
                }

                auto self(this->shared_from_this());
                boost::asio::async_write(
                    m_socket,
                    m_writeBuffer,
                    [this, self](boost::system::error_code _ec, std::size_t _bytesTransferred) {
                        try
                        {
                            if (!_ec)
                            {
                                LOG(MiscCommon::debug) << "Message successfully sent to " << remoteEndIDString() << " ("
                                                       << _bytesTransferred << " bytes)";

                                if (m_isShuttingDown)
                                {
                                    LOG(MiscCommon::info)
                                        << "Shutdown signal has been successfully sent to " << remoteEndIDString();
                                    stop();
                                }

                                // lock the modification of the container
                                {
                                    std::lock_guard<std::mutex> lock(m_mutexWriteBuffer);

                                    // Log write statistics
                                    this->logWriteMessages(m_writeBufferQueue);

                                    m_writeBuffer.clear();
                                    m_writeBufferQueue.clear();
                                }
                                // we might need to send more messages
                                writeMessage();
                            }
                            else if ((boost::asio::error::eof == _ec) || (boost::asio::error::connection_reset == _ec))
                            {
                                LOG(MiscCommon::debug)
                                    << "Disconnect is detected while on write message: " << _ec.message();
                                onDissconnect();
                            }
                            else
                            {
                                // don't show error if service is closed
                                if (m_started)
                                    LOG(MiscCommon::error)
                                        << "Error sending to " << remoteEndIDString() << ": " << _ec.message();
                                else
                                    LOG(MiscCommon::info)
                                        << "The stop signal is received, aborting current operation and "
                                           "closing the connection: "
                                        << _ec.message();
                                stop();
                            }
                        }
                        catch (std::exception& ex)
                        {
                            LOG(MiscCommon::error) << "BaseChannelImpl can't write message (callback): " << ex.what();
                        }
                    });
            }

            void onDissconnect()
            {
                LOG(MiscCommon::debug) << "The session was disconnected by the remote end: " << remoteEndIDString();
                // stopping the channel
                stop();

                // give a chance to children to execute something
                this->onEvent(EChannelEvents::OnRemoteEndDissconnected);

                // Call external event handler
                T* pThis = static_cast<T*>(this);
                if (m_disconnectEventHandler)
                    m_disconnectEventHandler(pThis);
            }

            bool isCmdAllowedWithoutHandshake(ECmdType _cmd)
            {
                if (m_isHandshakeOK)
                    return true;
                return (_cmd == cmdHANDSHAKE || _cmd == cmdREPLY_HANDSHAKE_OK || _cmd == cmdREPLY_HANDSHAKE_ERR);
            }

          private:
            void close()
            {
                m_socket.close();
            }

          private:
            handlerDisconnectEventFunction_t m_disconnectEventHandler;

          protected:
            bool m_isHandshakeOK;
            EChannelType m_channelType;

          private:
            boost::asio::io_service& m_io_service;
            boost::asio::ip::tcp::socket m_socket;
            bool m_started;
            CProtocolMessage::protocolMessagePtr_t m_currentMsg;

            protocolMessagePtrQueue_t m_writeQueue;
            protocolMessagePtrQueue_t m_writeQueueBeforeHandShake;

            std::mutex m_mutexWriteBuffer;
            protocolMessageBuffer_t m_writeBuffer;
            protocolMessagePtrQueue_t m_writeBufferQueue;

            // BinaryAttachment
            typedef std::map<boost::uuids::uuid, binaryAttachmentInfoPtr_t> binaryAttachmentMap_t;
            binaryAttachmentMap_t m_binaryAttachmentMap;
            std::mutex m_binaryAttachmentMutex;

            protocolMessagePtrQueue_t m_accumulativeWriteQueue;
            deadlineTimerPtr_t m_deadlineTimer;

            bool m_isShuttingDown;
        };
    }
}

#endif /* defined(__DDS__BaseChannelImpl__) */
