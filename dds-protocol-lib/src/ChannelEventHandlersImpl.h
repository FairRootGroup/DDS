// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_ChannelEventHandlersImpl_h
#define DDS_ChannelEventHandlersImpl_h

#include "BaseEventHandlersImpl.h"
#include "CommandAttachmentImpl.h"
#include "ProtocolCommands.h"

#define DECLARE_CHANNEL_EVENTS_INTERFACE               \
    using CChannelEventHandlersImpl::registerHandler;  \
    using CChannelEventHandlersImpl::dispatchHandlers; \
    using CChannelEventHandlersImpl::handlerExists;

#define REGISTER_CHANNEL_EVENT_HANDLER(eventID)                                                         \
  public:                                                                                               \
    template <EChannelEvents _cmd, typename func_t>                                                     \
    void registerHandler(                                                                               \
        func_t _handler,                                                                                \
        typename std::enable_if<std::is_same<std::integral_constant<EChannelEvents, _cmd>,              \
                                             std::integral_constant<EChannelEvents, eventID>>::value && \
                                std::is_same<func_t, std::function<void()>>::value>::type* = nullptr)   \
    {                                                                                                   \
        CBaseEventHandlersImpl<EChannelEvents>::registerHandlerImpl<_cmd>(_handler);                    \
    }

namespace dds
{
    namespace protocol_api
    {
        // Channel events, which channels and users of channel objects can subscribe on.
        enum class EChannelEvents
        {
            OnConnected,
            OnFailedToConnect,
            OnRemoteEndDissconnected,
            OnHandshakeOK,
            OnHandshakeFailed
        };

        class CChannelEventHandlersImpl : private CBaseEventHandlersImpl<EChannelEvents>
        {

          public:
            template <class... Args>
            void dispatchHandlers(EChannelEvents _cmd, Args&&... args)
            {
                CBaseEventHandlersImpl<EChannelEvents>::dispatchHandlersImpl<>(_cmd, std::forward<Args>(args)...);
            }

          public:
            bool handlerExists(EChannelEvents _cmd) const
            {
                return CBaseEventHandlersImpl<EChannelEvents>::handlerExistsImpl(_cmd);
            }

            REGISTER_CHANNEL_EVENT_HANDLER(EChannelEvents::OnConnected)
            REGISTER_CHANNEL_EVENT_HANDLER(EChannelEvents::OnFailedToConnect)
            REGISTER_CHANNEL_EVENT_HANDLER(EChannelEvents::OnRemoteEndDissconnected)
            REGISTER_CHANNEL_EVENT_HANDLER(EChannelEvents::OnHandshakeOK)
            REGISTER_CHANNEL_EVENT_HANDLER(EChannelEvents::OnHandshakeFailed)
        };
    }
}

#endif
