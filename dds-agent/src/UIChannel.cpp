// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "UIChannel.h"

using namespace std;
using namespace dds;
using namespace MiscCommon;

CUIChannel::CUIChannel(boost::asio::io_service& _service)
    : CConnectionImpl<CUIChannel>(_service)
    , m_isHandShakeOK(false)
    , m_type(EChannelType::UNDEFINED)
{
}

string CUIChannel::getTypeName() const
{
    return g_vecChannelType[static_cast<size_t>(m_type)];
}

std::string CUIChannel::_remoteEndIDString()
{
    switch (m_type)
    {
        case EChannelType::KEY_VALUE_GUARD:
            return "key-value-guard";
        default:
            return "\"no-name-end\"";
    }
}

bool CUIChannel::on_cmdHANDSHAKE_KEY_VALUE_GUARD(
    SCommandAttachmentImpl<cmdHANDSHAKE_KEY_VALUE_GUARD>::ptr_t _attachment)
{
    // send shutdown if versions are incompatible
    if (*_attachment != SVersionCmd())
    {
        m_isHandShakeOK = false;
        // Send reply that the version of the protocol is incompatible
        LOG(warning) << "Incompatible protocol version of the client: " << remoteEndIDString();
        pushMsg<cmdREPLY_ERR_BAD_PROTOCOL_VERSION>();
    }
    else
    {
        m_isHandShakeOK = true;
        m_type = EChannelType::KEY_VALUE_GUARD;
        // everything is OK, we can work with this agent
        LOG(info) << "key-value-guard [" << socket().remote_endpoint().address().to_string()
                  << "] has successfully connected.";

        pushMsg<cmdREPLY_HANDSHAKE_OK>();
    }
    return true;
}

bool CUIChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    return false; // the connection manager should process this message
}