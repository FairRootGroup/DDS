// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "CustomCmd.h"
#include "CustomCmdGuard.h"

using namespace dds;
using namespace dds::custom_cmd;
using namespace dds::custom_cmd_api;
using namespace dds::protocol_api;
using namespace MiscCommon;

CCustomCmd::~CCustomCmd()
{
    unsubscribe();
}

int CCustomCmd::sendCmd(const std::string& _command, const std::string& _condition)
{
    CCustomCmdGuard::instance().initAgentConnection();

    SCustomCmdCmd cmd;
    cmd.m_sCmd = _command;
    cmd.m_sCondition = _condition;
    if (1 == CCustomCmdGuard::instance().sendCmd(cmd))
        return 1;

    return 0;
}

void CCustomCmd::subscribeCmd(cmdSignal_t::slot_function_type _subscriber)
{
    CCustomCmdGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting for custom commands.";

    connection_t connection = CCustomCmdGuard::instance().connectCmd(_subscriber);
}

void CCustomCmd::subscribeReply(replySignal_t::slot_function_type _subscriber)
{
    CCustomCmdGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting for replys from custom commands.";

    connection_t connection = CCustomCmdGuard::instance().connectReply(_subscriber);
}

void CCustomCmd::unsubscribe()
{
    LOG(info) << "Unsubscribing from custom commands and reply events.";
    CCustomCmdGuard::instance().disconnect();
}