// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TalkToAgent.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

int CTalkToAgent::on_cmdHANDSHAKE(const CProtocolMessage& _msg)
{
    SVersionCmd ver;
    ver.convertFromData(_msg.bodyToContainer());
    // send shutdown if versions are incompatible
    if (ver != SVersionCmd())
    {
        m_isHandShakeOK = false;
        // Send reply that the version of the protocol is incompatible
        LOG(warning) << "Client's protocol version is incompatable. Client: "
                     << socket().remote_endpoint().address().to_string();
        CProtocolMessage msg;
        msg.encode_message(cmdREPLY_ERR_BAD_PROTOCOL_VERSION, BYTEVector_t());
        pushMsg(msg);
    }
    else
    {
        m_isHandShakeOK = true;
        // everything is OK, we can work with this agent
        LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                  << "] has succesfully connected.";
        CProtocolMessage msg;
        msg.encode_message(cmdREPLY_HANDSHAKE_OK, BYTEVector_t());
        pushMsg(msg);
        send();
    }
    return 0;
}

int CTalkToAgent::on_cmdSUBMIT(const CProtocolMessage& _msg)
{
    SSubmitCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());
    LOG(info) << "Recieved a Submit of the topo [" << cmd.m_sTopoFile
              << "] command from: " << socket().remote_endpoint().address().to_string();

    // TODO: Implement me. So far we always send OK
    CProtocolMessage msg;
    msg.encode_message(cmdREPLY_SUBMIT_OK, BYTEVector_t());
    pushMsg(msg);
    send();

    return 0;
}
