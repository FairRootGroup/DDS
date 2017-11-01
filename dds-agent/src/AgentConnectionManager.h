// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AGENT__AgentConnectionManager__
#define __DDS__AGENT__AgentConnectionManager__

// DDS
#include "CommanderChannel.h"
#include "Options.h"
#include "SMCommanderChannel.h"
#include "SMLeaderChannel.h"
#include "SMUIChannel.h"
// BOOST
#include <boost/asio.hpp>

namespace dds
{
    namespace agent_cmd
    {
        class CAgentConnectionManager : public std::enable_shared_from_this<CAgentConnectionManager>
        {
            typedef std::vector<pid_t> childrenPidContainer_t;
            typedef std::vector<protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t>
                updateKeyAttachmentQueue_t;

          public:
            CAgentConnectionManager(const SOptions_t& _options);
            virtual ~CAgentConnectionManager();

          public:
            void start();
            void stop();

          private:
            void startService();
            void createNetworkAgentChannel(uint64_t _protocolHeaderID);
            void createSMIntercomChannel(uint64_t _protocolHeaderID);
            void createSMLeaderChannel(uint64_t _protocolHeaderID);
            void createSMAgentChannel(uint64_t _protocolHeaderID);
            void doAwaitStop();
            void onNewUserTask(pid_t _pid);
            void terminateChildrenProcesses();
            void on_cmdSHUTDOWN(const protocol_api::SSenderInfo& _sender,
                                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
                                CSMCommanderChannel::weakConnectionPtr_t _channel);
            void on_cmdREPLY_LOBBY_MEMBER_HANDSHAKE_ERR(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_LOBBY_MEMBER_HANDSHAKE_ERR>::ptr_t
                    _attachment,
                CSMCommanderChannel::weakConnectionPtr_t _channel);
            void on_cmdUPDATE_KEY(const protocol_api::SSenderInfo& _sender,
                                  protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment,
                                  CSMCommanderChannel::weakConnectionPtr_t _channel);
            void on_cmdUPDATE_KEY_ERROR(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY_ERROR>::ptr_t _attachment,
                CSMCommanderChannel::weakConnectionPtr_t _channel);
            void on_cmdDELETE_KEY(const protocol_api::SSenderInfo& _sender,
                                  protocol_api::SCommandAttachmentImpl<protocol_api::cmdDELETE_KEY>::ptr_t _attachment,
                                  CSMCommanderChannel::weakConnectionPtr_t _channel);
            void on_cmdSIMPLE_MSG(const protocol_api::SSenderInfo& _sender,
                                  protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                                  CSMCommanderChannel::weakConnectionPtr_t _channel);
            void on_cmdSTOP_USER_TASK(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSTOP_USER_TASK>::ptr_t _attachment,
                CSMCommanderChannel::weakConnectionPtr_t _channel);
            void on_cmdCUSTOM_CMD(const protocol_api::SSenderInfo& _sender,
                                  protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment,
                                  CSMCommanderChannel::weakConnectionPtr_t _channel);

            // Messages from shared memory
            void on_cmdUPDATE_KEY_SM(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
            void on_cmdCUSTOM_CMD_SM(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment);

            void taskExited(int _pid, int _exitCode);

          private:
            boost::asio::io_service m_io_service;
            boost::thread_group m_workerThreads;

            CCommanderChannel::connectionPtr_t m_agent;
            CSMUIChannel::connectionPtr_t m_SMChannel;
            CSMCommanderChannel::connectionPtr_t m_SMAgent;
            CSMLeaderChannel::connectionPtr_t m_SMLeader;

            boost::asio::signal_set m_signals;
            SOptions_t m_options;
            childrenPidContainer_t m_children;
            std::mutex m_childrenContainerMutex;
            bool m_bStarted;
            bool m_isLeaderBasedDeployment;
        };
    }
}

#endif /* defined(__DDS__AGENT__AgentConnectionManager__) */
