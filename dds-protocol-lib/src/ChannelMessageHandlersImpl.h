// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_ChannelMessageHandlersImpl_h
#define DDS_ChannelMessageHandlersImpl_h
// DDS
#include "BaseEventHandlersImpl.h"
#include "ProtocolCommands.h"

namespace dds
{
    namespace protocol_api
    {

        typedef CBaseEventHandlersImpl<ECmdType> CChannelMessageHandlersImpl;
    }
}

#endif
