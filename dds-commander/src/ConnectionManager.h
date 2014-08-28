// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ConnectionManager__
#define __DDS__ConnectionManager__
// DDS
#include "ConnectionManagerImpl.h"
#include "AgentChannel.h"
// STD
#include <mutex>

namespace dds
{
    class CConnectionManager : public CConnectionManagerImpl<CAgentChannel, CConnectionManager>
    {
      public:
        CConnectionManager(const SOptions_t& _options,
                           boost::asio::io_service& _io_service,
                           boost::asio::ip::tcp::endpoint& _endpoint);

        ~CConnectionManager();

      public:
        void newClientCreated(CAgentChannel::connectionPtr_t _newClient);

      private:
        bool agentsInfoHandler(const CProtocolMessage& _msg, CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdGET_LOG(const CProtocolMessage& _msg, CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdBINARY_ATTACHMENT_LOG(const CProtocolMessage& _msg, CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdGET_LOG_ERROR(const CProtocolMessage& _msg, CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdSUBMIT(const CProtocolMessage& _msg, CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdSUBMIT_START(const CProtocolMessage& _msg, CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdSTART_DOWNLOAD_TEST(const CProtocolMessage& _msg, CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdDOWNLOAD_TEST_STAT(const CProtocolMessage& _msg, CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdDOWNLOAD_TEST_ERROR(const CProtocolMessage& _msg, CAgentChannel::weakConnectionPtr_t _channel);

        void checkAllLogsReceived();
        void checkAllDownloadTestsReceived();

        void sendTestBinaryAttachment(size_t _binarySize, CAgentChannel::connectionPtr_t _channel);

        struct SChannelInfo
        {
            SChannelInfo()
                : m_nofRequests(0)
                , m_nofReceived(0)
                , m_nofReceivedErrors(0)
                , m_mutexStart()
                , m_mutexReceive()
            {
            }

            size_t nofReceived() const
            {
                return (m_nofReceived + m_nofReceivedErrors);
            }

            bool allReceived() const
            {
                return (nofReceived() == m_nofRequests);
            }

            void zeroCounters()
            {
                m_nofRequests = 0;
                m_nofReceived = 0;
                m_nofReceivedErrors = 0;
            }

            size_t m_nofRequests;
            size_t m_nofReceived;
            size_t m_nofReceivedErrors;
            CAgentChannel::weakConnectionPtr_t m_channel;
            std::mutex m_mutexStart;
            std::mutex m_mutexReceive;
        };

        struct SDownloadStat
        {
            SDownloadStat()
                : m_totalReceived(0)
                , m_totalTime(0)
            {
            }
            size_t m_totalReceived; // [bytes]
            size_t m_totalTime;     // [ms]
        };

        SChannelInfo m_getLog;
        SChannelInfo m_downloadTest;
        SDownloadStat m_downloadTestStat;
        std::string m_sCurrentTopoFile;
    };
}
#endif /* defined(__DDS__ConnectionManager__) */
