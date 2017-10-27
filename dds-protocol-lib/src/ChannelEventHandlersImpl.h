// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_ChannelEventHandlersImpl_h
#define DDS_ChannelEventHandlersImpl_h

#include "BaseEventHandlersImpl.h"

namespace dds
{
    namespace protocol_api
    {
        /// Channel events, which channels and users of channel objects can subscribe on.
        enum class EChannelEvents
        {
            OnConnected,
            OnFailedToConnect,
            OnRemoteEndDissconnected,
            OnHandshakeOK,
            OnHandshakeFailed,
            OnNewUserTask,
            OnSMStart ///< Shared memory channel start
        };

        class CChannelEventHandlersImpl : private CBaseEventHandlersImpl<EChannelEvents>
        {
            DDS_BEGIN_EVENT_HANDLERS(EChannelEvents)
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnConnected,
                                       void(const protocol_api::SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnFailedToConnect,
                                       void(const protocol_api::SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnRemoteEndDissconnected,
                                       void(const protocol_api::SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnHandshakeOK,
                                       void(const protocol_api::SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnHandshakeFailed,
                                       void(const protocol_api::SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnNewUserTask,
                                       void(const protocol_api::SSenderInfo&, pid_t))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnSMStart,
                                       void(const protocol_api::SSenderInfo&))
            DDS_END_EVENT_HANDLERS
        };
    }
}

#endif
