// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AGENT__AgentConnectionManager__
#define __DDS__AGENT__AgentConnectionManager__

// DDS
#include "CommanderChannel.h"
#include "Options.h"

namespace dds
{
    namespace agent_cmd
    {
        class CAgentConnectionManager : public std::enable_shared_from_this<CAgentConnectionManager>
        {
            typedef std::vector<protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t>
                updateKeyAttachmentQueue_t;
            // map of task id to its pid
            //    using pidContainer_t = std::map<uint64_t, pid_t>;

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
            void doAwaitStop();
            //  void onNewUserTask(uint64_t _taskID, pid_t _pid);
            // void terminateChildrenProcesses(uint64_t _taskID = 0);
            void on_cmdSHUTDOWN(const protocol_api::SSenderInfo& _sender,
                                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
                                CCommanderChannel::weakConnectionPtr_t _channel);

            // void taskExited(uint64_t _taskID, int _pid, int _exitCode);

          private:
            boost::asio::io_context m_io_context;
            boost::thread_group m_workerThreads;

            CCommanderChannel::connectionPtr_t m_commanderChannel;

            boost::asio::signal_set m_signals;
            SOptions_t m_options;
            //    pidContainer_t m_taskPids;
            //   std::mutex m_taskPidsMutex;
            bool m_bStarted;
        };
    } // namespace agent_cmd
} // namespace dds

#endif /* defined(__DDS__AGENT__AgentConnectionManager__) */
