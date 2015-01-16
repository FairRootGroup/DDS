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
#include <chrono>
#include <condition_variable>
// BOOST
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#pragma clang diagnostic pop
// DDS
#include "CommandAttachmentImpl.h"
#include "Logger.h"
#include "MonitoringThread.h"

#define BEGIN_MSG_MAP(theClass)                                                                                      \
  public:                                                                                                            \
    friend CConnectionImpl<theClass>;                                                                                \
    void processMessage(CProtocolMessage::protocolMessagePtr_t _currentMsg)                                          \
    {                                                                                                                \
        using namespace dds;                                                                                         \
        CMonitoringThread::instance().updateIdle();                                                                  \
        bool processed = true;                                                                                       \
        ECmdType currentCmd = static_cast<ECmdType>(_currentMsg->header().m_cmd);                                    \
        if (currentCmd == cmdBINARY_ATTACHMENT)                                                                      \
        {                                                                                                            \
            typedef typename SCommandAttachmentImpl<cmdBINARY_ATTACHMENT>::ptr_t attahcmentPtr_t;                    \
            attahcmentPtr_t attachmentPtr = SCommandAttachmentImpl<cmdBINARY_ATTACHMENT>::decode(_currentMsg);       \
            processBinaryAttachmentCmd(attachmentPtr);                                                               \
            return;                                                                                                  \
        }                                                                                                            \
        if (currentCmd == cmdBINARY_ATTACHMENT_START)                                                                \
        {                                                                                                            \
            typedef typename SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_START>::ptr_t attahcmentPtr_t;              \
            attahcmentPtr_t attachmentPtr = SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_START>::decode(_currentMsg); \
            processBinaryAttachmentStartCmd(attachmentPtr);                                                          \
            return;                                                                                                  \
        }                                                                                                            \
        switch (currentCmd)                                                                                          \
        {

#define MESSAGE_HANDLER(msg, func)                                                                                 \
    case msg:                                                                                                      \
    {                                                                                                              \
        typedef typename SCommandAttachmentImpl<msg>::ptr_t attahcmentPtr_t;                                       \
        attahcmentPtr_t attachmentPtr = SCommandAttachmentImpl<msg>::decode(_currentMsg);                          \
        LOG(MiscCommon::debug) << "Processing " << g_cmdToString[msg] << " received from " << remoteEndIDString(); \
        processed = func(attachmentPtr);                                                                           \
        if (!processed)                                                                                            \
        {                                                                                                          \
            if (m_registeredMessageHandlers.count(msg) == 0)                                                       \
            {                                                                                                      \
                LOG(MiscCommon::error) << "The received message was not processed and has no registered handler: " \
                                       << _currentMsg->toString();                                                 \
            }                                                                                                      \
            else                                                                                                   \
            {                                                                                                      \
                dispatch(msg, m_registeredMessageHandlers, attachmentPtr, this);                                   \
            }                                                                                                      \
        }                                                                                                          \
        break;                                                                                                     \
    }

#define END_MSG_MAP()                                                                                         \
    default:                                                                                                  \
        LOG(MiscCommon::error) << "The received message doesn't have a handler: " << _currentMsg->toString(); \
        }                                                                                                     \
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

#define REGISTER_ALL_DEFAULT_CALLBACKS                                             \
    REGISTER_DEFAULT_ON_CONNECT_CALLBACKS REGISTER_DEFAULT_ON_DISCONNECT_CALLBACKS \
        REGISTER_DEFAULT_ON_HEADER_READ_CALLBACKS

#define REGISTER_DEFAULT_REMOTE_ID_STRING \
    std::string _remoteEndIDString()      \
    {                                     \
        return "DDS Server";              \
    }

namespace dds
{
    // --- Helpers for events dispatching ---
    // TODO: Move to a seporate header
    struct SHandlerHlpFunc
    {
    };
    template <typename T>
    struct SHandlerHlpBaseFunc : SHandlerHlpFunc
    {
        T m_function;

        SHandlerHlpBaseFunc(T _function)
            : m_function(_function)
        {
        }
    };

    // Generic container of listeners for any type of function
    typedef std::multimap<ECmdType, std::unique_ptr<SHandlerHlpFunc>> Listeners_t;

    template <ECmdType _cmd, typename Func>
    static void addListener(Listeners_t& _listeners, Func _function)
    {
        std::unique_ptr<SHandlerHlpFunc> func_ptr(new SHandlerHlpBaseFunc<Func>(_function));
        _listeners.insert(Listeners_t::value_type(_cmd, std::move(func_ptr)));
    }

    template <typename... Args>
    static void callListeners(ECmdType _cmd, const Listeners_t& listeners, Args&&... args)
    {
        // typedef bool Func(Args...);
        typedef std::function<bool(Args...)> Func_t;
        auto functions = listeners.equal_range(_cmd);
        for (auto it = functions.first; it != functions.second; ++it)
        {
            const SHandlerHlpFunc& f = *it->second;
            Func_t func = static_cast<const SHandlerHlpBaseFunc<Func_t>&>(f).m_function;
            func(std::forward<Args>(args)...);
        }
    }

    template <class... Args>
    void dispatch(ECmdType _cmd, Listeners_t& _listeneres, Args&&... args)
    {
        callListeners(_cmd, _listeneres, std::forward<Args>(args)...);
    }
    // ------------------------------------

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
    class CConnectionImpl : public boost::noncopyable
    {
        typedef std::function<void(T*)> handlerDisconnectEventFunction_t;
        typedef std::deque<CProtocolMessage::protocolMessagePtr_t> protocolMessagePtrQueue_t;
        typedef std::vector<boost::asio::mutable_buffer> protocolMessageBuffer_t;

      public:
        typedef std::shared_ptr<T> connectionPtr_t;
        typedef std::weak_ptr<T> weakConnectionPtr_t;
        typedef std::vector<connectionPtr_t> connectionPtrVector_t;
        typedef std::vector<weakConnectionPtr_t> weakConnectionPtrVector_t;

      protected:
        CConnectionImpl<T>(boost::asio::io_service& _service)
            : m_io_service(_service)
            , m_socket(_service)
            , m_started(false)
            , m_currentMsg(std::make_shared<CProtocolMessage>())
            , m_binaryAttachmentMap()
            , m_binaryAttachmentMutex()
        {
        }

      public:
        ~CConnectionImpl<T>()
        {
            stop();
        }

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

        bool isWriteInProgress() const
        {
            return (!m_writeMsgBufferQueue.empty() || !m_writeMsgQueue.empty());
        }

        template <ECmdType _cmd, class A>
        void pushMsg(const A& _attachment)
        {
            CProtocolMessage::protocolMessagePtr_t msg = SCommandAttachmentImpl<_cmd>::encode(_attachment);

            bool bWriteInProgress(true);
            {
                // it can be called from multiple IO threads
                std::lock_guard<std::mutex> lock(m_mutex);
                // Do not execute writeMessage if older messages are being processed.
                // We must not register more, than async_write in the same time (different threads).
                // Only one async_write is allowed at time per socket to avoid messages corruption.
                bWriteInProgress = isWriteInProgress();
                m_writeMsgQueue.push_back(msg);
                LOG(MiscCommon::debug) << "MESSAGE QUEUE SIZE = " << m_writeMsgQueue.size()
                                       << "; SENDING BUFFER SIZE = " << m_writeMsgBufferQueue.size();
                LOG(MiscCommon::debug) << "pushMsg: " << (bWriteInProgress
                                                              ? "buffer messages, while sending is still in progress"
                                                              : "will send");
            }
            if (!bWriteInProgress)
            {
                // We need to make sure that write is not called from different threads.
                // Only one write at time is allowed by asio.
                // We therefore notify syncWrite about a chance to write
                m_cvReadyToWrite.notify_all();

                // process standard async writing
                writeMessage();
            }
        }

        template <ECmdType _cmd>
        void pushMsg()
        {
            SEmptyCmd cmd;
            pushMsg<_cmd>(cmd);
        }

        template <ECmdType _cmd, class A>
        void syncPushMsg(const A& _attachment)
        {
            CProtocolMessage::protocolMessagePtr_t msg = SCommandAttachmentImpl<_cmd>::encode(_attachment);
            syncWriteMessage(msg);
        }

        template <ECmdType _cmd>
        void syncPushMsg()
        {
            SEmptyCmd cmd;
            syncPushMsg<_cmd>(cmd);
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

        void pushBinaryAttachmentCmd(const std::string& _srcFilePath, const std::string& _fileName, uint16_t _cmdSource)
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
            syncPushMsg<cmdBINARY_ATTACHMENT_START>(start_cmd);

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
                    LOG(MiscCommon::error) << "Received binary attachment [" << fileId
                                           << "] which does not exist. Skip this message.";
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
                        ss << "Received binary file [" << fileId << "] has wrong CRC32 checksum: " << crc32.checksum()
                           << " instead of " << _attachment->m_crc32;
                        LOG(MiscCommon::error) << ss.str();
                        sendYourself<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, info->m_srcCommand));
                        return;
                    }

                    const std::string dir(CUserDefaults::getDDSPath());
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

        template <ECmdType _cmd, typename Func>
        void registerMessageHandler(Func _handler)
        {
            addListener<_cmd, Func>(m_registeredMessageHandlers, _handler);
        }

        void registerDissconnectEventHandler(handlerDisconnectEventFunction_t _handler)
        {
            m_dissconnectEventHandler = _handler;
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
                ss << pThis->_remoteEndIDString() << " [" << socket().remote_endpoint().address().to_string() << "]";
                return ss.str();
            }
            catch (...)
            {
                return std::string();
            }
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
                                               pushMsg<cmdHANDSHAKE>(cmd);
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
            boost::asio::async_read(
                m_socket,
                boost::asio::buffer(m_currentMsg->data(), CProtocolMessage::header_length),
                [this](boost::system::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        LOG(MiscCommon::debug) << "Received message HEADER from " << remoteEndIDString() << ": "
                                               << length << " bytes, expected " << CProtocolMessage::header_length;
                    }
                    if (!ec && m_currentMsg->decode_header())
                    {
                        //                    // give a chance to child to execute something
                        //                    T* pThis = static_cast<T*>(this);
                        //                    pThis->onHeaderRead();

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
                            LOG(MiscCommon::info) << "The stop signal is received, aborting current operation and "
                                                     "closing the connection: " << ec.message();

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
                // Read next message
                m_currentMsg->clear();
                readHeader();
                return;
            }

            boost::asio::async_read(
                m_socket,
                boost::asio::buffer(m_currentMsg->body(), m_currentMsg->body_length()),
                [this](boost::system::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        LOG(MiscCommon::debug) << "Received message BODY from " << remoteEndIDString() << " (" << length
                                               << " bytes): " << m_currentMsg->toString();

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
                            LOG(MiscCommon::info) << "The stop signal is received, aborting current operation and "
                                                     "closing the connection: " << ec.message();
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
            size_t nMsgSize(0);
            while (nMsgSize < 20000) // TODO: fix the "magic" size of the buffer (so far we sent a max ~20 KB message)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_writeMsgQueue.empty())
                    break;

                LOG(MiscCommon::debug) << "Sending to " << remoteEndIDString()
                                       << " a message: " << m_writeMsgQueue.front()->toString();

                nMsgSize += m_writeMsgQueue.front()->length();

                m_writeMsgBuffer.push_back(
                    boost::asio::buffer(m_writeMsgQueue.front()->data(), m_writeMsgQueue.front()->length()));
                m_writeMsgBufferQueue.push_back(m_writeMsgQueue.front());

                m_writeMsgQueue.pop_front();
            }

            boost::asio::async_write(
                m_socket,
                m_writeMsgBuffer,
                [this](boost::system::error_code _ec, std::size_t _bytesTransferred)
                {
                    if (!_ec)
                    {
                        LOG(MiscCommon::debug) << "Message successfully sent to " << remoteEndIDString() << " ("
                                               << _bytesTransferred << " bytes)";

                        // lock the modification of the container
                        std::unique_lock<std::mutex> lock(m_mutex);
                        m_writeMsgBuffer.clear();
                        m_writeMsgBufferQueue.clear();

                        // process next message if the queue is not empty
                        if (!m_writeMsgQueue.empty())
                        {
                            lock.unlock(); // avoid to dead-lock
                            writeMessage();
                        }
                        else
                        {
                            // We need to make sure that write is not called from different threads.
                            // Only one write at time is allowed by asio.
                            // We therefore notify syncWrite about a chance to write
                            m_cvReadyToWrite.notify_all();
                        }
                    }
                    else if ((boost::asio::error::eof == _ec) || (boost::asio::error::connection_reset == _ec))
                    {
                        onDissconnect();
                    }
                    else
                    {
                        // don't show error if service is closed
                        if (m_started)
                            LOG(MiscCommon::error) << "Error sending to " << remoteEndIDString() << ": "
                                                   << _ec.message();
                        else
                            LOG(MiscCommon::info) << "The stop signal is received, aborting current operation and "
                                                     "closing the connection: " << _ec.message();
                        stop();
                    }
                });
        }

        void syncWriteMessage(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (isWriteInProgress())
                m_cvReadyToWrite.wait(lock);

            LOG(MiscCommon::debug) << "Sending to " << remoteEndIDString() << _msg->toString();
            boost::system::error_code ec;
            size_t bytesTransfered = boost::asio::write(
                m_socket, boost::asio::buffer(_msg->data(), _msg->length()), boost::asio::transfer_all(), ec);

            writeHandler(ec, bytesTransfered);
        }

        void writeHandler(boost::system::error_code _ec, std::size_t _bytesTransferred)
        {
            if (!_ec)
            {
                LOG(MiscCommon::debug) << "Message successfully sent to " << remoteEndIDString() << " ("
                                       << _bytesTransferred << " bytes)";
            }
            else if ((boost::asio::error::eof == _ec) || (boost::asio::error::connection_reset == _ec))
            {
                onDissconnect();
            }
            else
            {
                // don't show error if service is closed
                if (m_started)
                    LOG(MiscCommon::error) << "Error sending to " << remoteEndIDString() << ": " << _ec.message();
                else
                    LOG(MiscCommon::info)
                        << "The stop signal is received, aborting current operation and closing the connection: "
                        << _ec.message();
                stop();
            }
        }

        void onDissconnect()
        {
            LOG(MiscCommon::debug) << "The session was disconnected by the remote end: " << remoteEndIDString();
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

      protected:
        Listeners_t m_registeredMessageHandlers;
        handlerDisconnectEventFunction_t m_dissconnectEventHandler;

      private:
        boost::asio::io_service& m_io_service;
        boost::asio::ip::tcp::socket m_socket;
        bool m_started;
        CProtocolMessage::protocolMessagePtr_t m_currentMsg;
        protocolMessagePtrQueue_t m_writeMsgQueue;
        protocolMessageBuffer_t m_writeMsgBuffer;
        protocolMessagePtrQueue_t m_writeMsgBufferQueue;
        std::mutex m_mutex;
        std::condition_variable m_cvReadyToWrite;

        // BinaryAttachment
        typedef std::map<boost::uuids::uuid, binaryAttachmentInfoPtr_t> binaryAttachmentMap_t;
        binaryAttachmentMap_t m_binaryAttachmentMap;
        std::mutex m_binaryAttachmentMutex;
    };
}

#endif /* defined(__DDS__Connection__) */
