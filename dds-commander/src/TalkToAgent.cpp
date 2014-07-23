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
        isHandShakeOK = false;
        // Send reply that the version of the protocol is incompatible
        LOG(warning) << "Client's protocol version is incompatable. Client: "
                     << socket().remote_endpoint().address().to_string();
        CProtocolMessage msg;
        msg.encode_message(cmdREPLY_ERR_BAD_PROTOCOL_VERSION, BYTEVector_t());
        pushMsg(msg);
    }
    else
    {
        isHandShakeOK = true;
        // everything is OK, we can work with this agent
        LOG(warning) << "The Agent [" << socket().remote_endpoint().address().to_string()
                     << "] has succesfully connected.";
        CProtocolMessage msg;
        msg.encode_message(cmdREPLY_HANDSHAKE_OK, BYTEVector_t());
        pushMsg(msg);
    }
    return 0;
}
