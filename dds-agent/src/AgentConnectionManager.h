// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AgentConnectionManager__
#define __DDS__AgentConnectionManager__

// DDS
#include "CommanderChannel.h"
#include "Options.h"
#include "UIConnectionManager.h"
// BOOST
#include <boost/asio.hpp>

namespace dds
{
    class CAgentConnectionManager
    {
        typedef std::vector<pid_t> childrenPidContainer_t;
        typedef std::shared_ptr<CUIConnectionManager> UIConnectionManagerPtr_t;

      public:
        CAgentConnectionManager(const SOptions_t& _options, boost::asio::io_service& _io_service);
        virtual ~CAgentConnectionManager();

      public:
        void start();
        void stop();

      private:
        void doAwaitStop();
        void onNewUserTask(pid_t _pid);
        void terminateChildrenProcesses();
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                            CCommanderChannel::weakConnectionPtr_t _channel);
        bool on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                              CCommanderChannel::weakConnectionPtr_t _channel);
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                              CCommanderChannel::weakConnectionPtr_t _channel);

      private:
        boost::asio::io_service& m_service;
        boost::asio::signal_set m_signals;
        dds::SOptions_t m_options;
        CCommanderChannel::connectionPtr_t m_agents;
        childrenPidContainer_t m_children;
        std::mutex m_childrenContainerMutex;
        bool m_bStarted;
        boost::asio::io_service m_UI_io_service;
        boost::asio::ip::tcp::endpoint m_UI_end_point;
        UIConnectionManagerPtr_t m_UIConnectionMng;
    };
}

#endif /* defined(__DDS__AgentConnectionManager__) */
