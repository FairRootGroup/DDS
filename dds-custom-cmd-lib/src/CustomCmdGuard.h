// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_CustomCmdGuard_h
#define DDS_CustomCmdGuard_h
// DDS
#include "AgentConnectionManager.h"
// STD
#include <string>
#include <vector>
// BOOST
#include <boost/signals2/signal.hpp>

namespace dds
{
    namespace custom_cmd_api
    {
        typedef boost::signals2::signal<void(const std::string&, const std::string&, uint64_t)> cmdSignal_t;
        typedef boost::signals2::signal<void(const std::string&)> replySignal_t;
        typedef boost::signals2::connection connection_t;

        struct SSyncHelper
        {
            cmdSignal_t m_cmdSignal;
            replySignal_t m_replySignal;
        };

        class CCustomCmdGuard
        {
            typedef std::shared_ptr<CAgentConnectionManager> AgentConnectionManagerPtr_t;

          private:
            CCustomCmdGuard();
            ~CCustomCmdGuard();

          public:
            static CCustomCmdGuard& instance();

            connection_t connectCmd(cmdSignal_t::slot_function_type _subscriber);
            connection_t connectReply(replySignal_t::slot_function_type _subscriber);
            void disconnect();

            int sendCmd(const protocol_api::SCustomCmdCmd& _command);

            void initAgentConnection();

          public:
            SSyncHelper m_syncHelper;

          private:
            AgentConnectionManagerPtr_t m_agentConnectionMng;
            std::mutex m_initAgentConnectionMutex;
        };
    }
}

#endif
