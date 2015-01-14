// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "UIChannel.h"

using namespace std;
using namespace dds;
using namespace MiscCommon;

CUIChannel::CUIChannel(boost::asio::io_service& _service)
    : CServerChannelImpl<CUIChannel>(_service, { EChannelType::KEY_VALUE_GUARD })
{
}

std::string CUIChannel::_remoteEndIDString()
{
    return "key-value-guard";
}

void CUIChannel::onHandshakeOK()
{
}

void CUIChannel::onHandshakeERR()
{
}

bool CUIChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    return false; // the connection manager should process this message
}
