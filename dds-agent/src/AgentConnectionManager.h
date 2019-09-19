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
#include "SMIntercomChannel.h"
#include "SMLeaderChannel.h"
#include "TopoCore.h"

namespace dds
{
    namespace agent_cmd
    {
        class CAgentConnectionManager : public std::enable_shared_from_this<CAgentConnectionManager>
        {
            typedef std::vector<protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t>
                updateKeyAttachmentQueue_t;

          public:
            CAgentConnectionManager(const SOptions_t& _options);
            virtual ~CAgentConnectionManager();

          public:
            void start();
            void stop();

          private:
            void startService(size_t _numThreads);
            void createCommanderChannel(uint64_t _protocolHeaderID);
            void createSMIntercomChannel(uint64_t _protocolHeaderID);
            void createSMLeaderChannel(uint64_t _protocolHeaderID);
            void createSMCommanderChannel(uint64_t _protocolHeaderID);
            void doAwaitStop();
            void onNewUserTask(pid_t _pid);
            void terminateChildrenProcesses();
            void on_cmdSHUTDOWN(const protocol_api::SSenderInfo& _sender,
                                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
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
            void on_cmdBINARY_ATTACHMENT_RECEIVED(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
                CSMCommanderChannel::weakConnectionPtr_t _channel);

            void taskExited(int _pid, int _exitCode);

            void send_cmdUPDATE_KEY(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);

          private:
            boost::asio::io_context m_io_context;
            boost::thread_group m_workerThreads;

            CCommanderChannel::connectionPtr_t m_commanderChannel;
            CSMIntercomChannel::connectionPtr_t m_SMIntercomChannel;
            CSMCommanderChannel::connectionPtr_t m_SMCommanderChannel;
            CSMLeaderChannel::connectionPtr_t m_SMLeaderChannel;

            boost::asio::signal_set m_signals;
            SOptions_t m_options;
            pid_t m_taskPid;
            std::mutex m_taskPidMutex;
            bool m_bStarted;
            topology_api::CTopoCore m_topo;
        };
    } // namespace agent_cmd
} // namespace dds

#endif /* defined(__DDS__AGENT__AgentConnectionManager__) */
