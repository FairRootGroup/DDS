// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AgentConnectionManager__
#define __DDS__AgentConnectionManager__

// BOOST
#include <boost/asio.hpp>
// DDS
#include "CommanderChannel.h"
#include "Options.h"

namespace dds
{
    class CAgentConnectionManager
    {
        typedef std::vector<pid_t> childrenPidContainer_t;

      public:
        CAgentConnectionManager(const SOptions_t& _options, boost::asio::io_service& _service);
        virtual ~CAgentConnectionManager();

        void start();
        void stop();

      private:
        void doAwaitStop();
        void onNewUserTask(pid_t _pid);
        void terminateChildrenProcesses();
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                            CCommanderChannel::weakConnectionPtr_t _channel);

      private:
        boost::asio::io_service& m_service;
        boost::asio::signal_set m_signals;
        dds::SOptions_t m_options;
        CCommanderChannel::connectionPtrVector_t m_agents;
        childrenPidContainer_t m_children;
        std::mutex m_childrenContainerMutex;
        bool m_bStarted;
    };
}

#endif /* defined(__DDS__AgentConnectionManager__) */
