// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ConnectionManager__
#define __DDS__ConnectionManager__
// DDS
#include "ConnectionManagerImpl.h"
#include "AgentChannel.h"
#include "UIChannelInfo.h"
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
        bool agentsInfoHandler(CProtocolMessage::protocolMessagePtr_t _msg,
                               CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdGET_LOG(CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdBINARY_ATTACHMENT_LOG(CProtocolMessage::protocolMessagePtr_t _msg,
                                         CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdSUBMIT(CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdACTIVATE_AGENT(CProtocolMessage::protocolMessagePtr_t _msg,
                                  CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdSTART_DOWNLOAD_TEST(CProtocolMessage::protocolMessagePtr_t _msg,
                                       CAgentChannel::weakConnectionPtr_t _channel);
        bool on_cmdDOWNLOAD_TEST_STAT(CProtocolMessage::protocolMessagePtr_t _msg,
                                      CAgentChannel::weakConnectionPtr_t _channel);

        bool on_cmdSIMPLE_MSG(CProtocolMessage::protocolMessagePtr_t _msg, CAgentChannel::weakConnectionPtr_t _channel);

        CProtocolMessage::protocolMessagePtr_t getTestBinaryAttachment(size_t _binarySize);

        CGetLogChannelInfo m_getLog;
        CTestChannelInfo m_downloadTest;
        CActivateAgentsChannelInfo m_ActivateAgents;
        std::string m_sCurrentTopoFile;
    };
}
#endif /* defined(__DDS__ConnectionManager__) */
