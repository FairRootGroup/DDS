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
        LOG(warning) << "Client's protocol version is incompatable. Client: " << socket().remote_endpoint().address().to_string();
        // CProtocolMessage msg;
        // msg.encode_message(cmdSHUTDOWN, BYTEVector_t());
    }
    return 0;
}
