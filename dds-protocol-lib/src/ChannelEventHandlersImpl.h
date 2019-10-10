// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_ChannelEventHandlersImpl_h
#define DDS_ChannelEventHandlersImpl_h

#include "BaseEventHandlersImpl.h"
#include "ChannelInfo.h"

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
            OnAssignUserTask,
            OnSMStart, ///< Shared memory channel start
            OnReplyAddSlot,
            OnChangeChannelInfo
        };

        class CChannelEventHandlersImpl : private CBaseEventHandlersImpl<EChannelEvents>
        {
            DDS_BEGIN_EVENT_HANDLERS(EChannelEvents)
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents, EChannelEvents::OnConnected, void(const SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents, EChannelEvents::OnFailedToConnect, void(const SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnRemoteEndDissconnected,
                                       void(const SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents, EChannelEvents::OnHandshakeOK, void(const SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents, EChannelEvents::OnHandshakeFailed, void(const SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents, EChannelEvents::OnAssignUserTask, void(const SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents, EChannelEvents::OnSMStart, void(const SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents, EChannelEvents::OnReplyAddSlot, void(const SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents, EChannelEvents::OnChangeChannelInfo, void(const SSenderInfo&))
            DDS_END_EVENT_HANDLERS
        };
    } // namespace protocol_api
} // namespace dds

#endif
