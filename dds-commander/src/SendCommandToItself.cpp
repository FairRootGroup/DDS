// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SendCommandToItself.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

void CSendCommandToItself::setTopoFile(const string& _topoFile)
{
    m_sTopoFile = _topoFile;
}

int CSendCommandToItself::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    // Create the command's attachment
    SSubmitCmd cmd;
    cmd.m_sTopoFile = m_sTopoFile;
    BYTEVector_t data;
    cmd.convertToData(&data);

    CProtocolMessage msg;
    msg.encode_message(cmdSUBMIT, data);
    pushMsg(msg);
    send();

    return 0;
}

int CSendCommandToItself::on_cmdSIMPLE_MSG(const CProtocolMessage& _msg)
{
    return 0;
}

int CSendCommandToItself::on_cmdREPLY_SUBMIT_OK(const CProtocolMessage& _msg)
{
    return 0;
}

int CSendCommandToItself::on_cmdREPLY_ERR_SUBMIT(const CProtocolMessage& _msg)
{
    return 0;
}
