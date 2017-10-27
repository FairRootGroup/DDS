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
            CAgentConnectionManager(const SOptions_t& _options, boost::asio::io_service& _io_service);
            virtual ~CAgentConnectionManager();

          public:
            void start();
            void stop();

          private:
            void createAndStartNetworkAgentChannel(uint64_t _protocolHeaderID);
            void createAndStartSMIntercomChannel(uint64_t _protocolHeaderID);
            void createAndStartSMLeaderChannel(uint64_t _protocolHeaderID);
            void createAndStartSMAgentChannel(uint64_t _protocolHeaderID, bool _block);
            void doAwaitStop();
            void onNewUserTask(pid_t _pid);
            void terminateChildrenProcesses();
            void on_cmdSHUTDOWN(const protocol_api::SSenderInfo& _sender,
                                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
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
            boost::asio::io_service& m_service;
            boost::asio::signal_set m_signals;
            SOptions_t m_options;
            CCommanderChannel::connectionPtr_t m_agent;
            CSMUIChannel::connectionPtr_t m_SMChannel;
            CSMCommanderChannel::connectionPtr_t m_SMAgent;
            CSMLeaderChannel::connectionPtr_t m_SMLeader;
            childrenPidContainer_t m_children;
            std::mutex m_childrenContainerMutex;
            bool m_bStarted;
            boost::thread_group m_workerThreads;
            bool m_isLeaderBasedDeployment;
        };
    }
}

#endif /* defined(__DDS__AGENT__AgentConnectionManager__) */
