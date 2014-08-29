// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "GetLogChannel.h"
// BOOST
//#include <boost/crc.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace std;

bool CGetLogChannel::on_cmdREPLY_HANDSHAKE_OK(CProtocolMessage::protocolMessagePtr_t _msg)
{
    m_isHandShakeOK = true;

    pushMsg<cmdGET_LOG>();

    return true;
}

bool CGetLogChannel::on_cmdLOG_RECIEVED(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    return true;
}

bool CGetLogChannel::on_cmdALL_LOGS_RECIEVED(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    stop();
    exit(EXIT_SUCCESS);
}

bool CGetLogChannel::on_cmdGET_LOG_ERROR(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    return true;
}

bool CGetLogChannel::on_cmdGET_LOG_FATAL(CProtocolMessage::protocolMessagePtr_t _msg)
{
    SSimpleMsgCmd recieved_cmd;
    recieved_cmd.convertFromData(_msg->bodyToContainer());
    LOG(log_stdout) << recieved_cmd.m_sMsg;

    stop();
    exit(EXIT_FAILURE);
}