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

        //class CChannelMessageHandlersImpl : public CBaseEventHandlersImpl<ECmdType>
        //{
            // void registerHandler(ECmdType _cmd, std::function<bool(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t)
            //{
            //    CBaseEventHandlersImpl<ECmdType>::registerHandler(_cmd, _handler);
            //}
        //};
    }
}

#endif
