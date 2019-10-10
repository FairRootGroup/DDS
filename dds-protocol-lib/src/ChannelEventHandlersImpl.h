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
            OnAssignUserTask,
            OnSMStart, ///< Shared memory channel start
            OnReplyAddSlot,
            //            OnLobbyMemberHandshakeFailed,
            //            OnLobbyMemberInfo
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
                                       void(const protocol_api::SSenderInfo&, uint64_t, pid_t))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnAssignUserTask,
                                       void(const protocol_api::SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnSMStart,
                                       void(const protocol_api::SSenderInfo&))
            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
                                       EChannelEvents::OnReplyAddSlot,
                                       void(const protocol_api::SSenderInfo&))
            //            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
            //                                       EChannelEvents::OnLobbyMemberHandshakeFailed,
            //                                       void(const protocol_api::SSenderInfo&))
            //            DDS_REGISTER_EVENT_HANDLER(EChannelEvents,
            //                                       EChannelEvents::OnLobbyMemberInfo,
            //                                       void(const protocol_api::SSenderInfo&, const std::string&))
            DDS_END_EVENT_HANDLERS
        };
    } // namespace protocol_api
} // namespace dds

#endif
