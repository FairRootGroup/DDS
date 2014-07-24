// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TalkToCommander.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

CTalkToCommander::CTalkToCommander(boost::asio::io_service& _service)
    : CConnectionImpl<CTalkToCommander>(_service)
    , m_isHandShakeOK(false)
{
}

int CTalkToCommander::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    // Create the command's attachment
    /*    SSubmitCmd cmd;
        cmd.m_sTopoFile = m_sTopoFile;
        BYTEVector_t data;
        cmd.convertToData(&data);

        CProtocolMessage msg;
        msg.encode_message(cmdSUBMIT, data);
        pushMsg(msg);
        send();
    */
    return 0;
}

int CTalkToCommander::on_cmdSIMPLE_MSG(const CProtocolMessage& _msg)
{
    return 0;
}