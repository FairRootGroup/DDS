// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "CustomCmdGuard.h"
#include "UserDefaults.h"
#include "BOOST_FILESYSTEM.h"

using namespace std;
using namespace dds;
using namespace dds::user_defaults_api;
using namespace dds::custom_cmd_api;
using namespace dds::protocol_api;
using namespace MiscCommon;

CCustomCmdGuard::CCustomCmdGuard()
{
    Logger::instance().init(); // Initialize log
}

CCustomCmdGuard::~CCustomCmdGuard()
{
}

CCustomCmdGuard& CCustomCmdGuard::instance()
{
    static CCustomCmdGuard instance;
    return instance;
}

connection_t CCustomCmdGuard::connectCmd(cmdSignal_t::slot_function_type _subscriber)
{
    return m_syncHelper.m_cmdSignal.connect(_subscriber);
}

connection_t CCustomCmdGuard::connectReply(replySignal_t::slot_function_type _subscriber)
{
    return m_syncHelper.m_replySignal.connect(_subscriber);
}

void CCustomCmdGuard::disconnect()
{
    m_syncHelper.m_cmdSignal.disconnect_all_slots();
    m_syncHelper.m_replySignal.disconnect_all_slots();
}

void CCustomCmdGuard::initAgentConnection()
{
    lock_guard<std::mutex> lock(m_initAgentConnectionMutex);

    LOG(info) << "CCustomCmdGuard::initAgentConnection: is going to init";

    if (!m_agentConnectionMng)
    {
        LOG(info) << "CCustomCmdGuard::initAgentConnection: start init";

        m_agentConnectionMng.reset();
        m_agentConnectionMng = make_shared<CAgentConnectionManager>();
        m_agentConnectionMng->m_syncHelper = &m_syncHelper;

        try
        {
            m_agentConnectionMng->start();
        }
        catch (exception& _e)
        {
            LOG(fatal) << "AgentConnectionManager: exception in the transport service: " << _e.what();
        }
    }
}

int CCustomCmdGuard::sendCmd(const protocol_api::SCustomCmdCmd& _command)
{
    if (!m_agentConnectionMng)
    {
        LOG(error)
            << "CCustomCmdGuard::sendCmd: Agent connection channel is not running. Failed to send custom command "
            << _command;
        return 1;
    }

    LOG(info) << "CCustomCmdGuard::sendCmd: sending custom command: " << _command;
    return m_agentConnectionMng->sendCmd(_command);
}
