// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "UIChannel.h"

using namespace dds;

CUIChannel::CUIChannel(boost::asio::io_service& _service)
    : CConnectionImpl<CUIChannel>(_service)
    , m_isHandShakeOK(false)
    , m_type(EChannelType::UNDEFINED)
{
}

std::string CUIChannel::getTypeName() const
{
    return g_vecChannelType[static_cast<size_t>(m_type)];
}