// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_ChannelMessageHandlersImpl_h
#define DDS_ChannelMessageHandlersImpl_h
// DDS
#include "BaseEventHandlersImpl.h"
#include "CommandAttachmentImpl.h"
#include "ProtocolCommands.h"

#define REGISTER_EVENT_HANDLER(eventID)                                                                               \
  public:                                                                                                             \
    template <ECmdType _cmd, typename func_t>                                                                         \
    void registerHandler(                                                                                             \
        func_t _handler,                                                                                              \
        typename std::enable_if<                                                                                      \
            std::is_same<std::integral_constant<ECmdType, _cmd>, std::integral_constant<ECmdType, eventID>>::value && \
            std::is_same<func_t, std::function<void(SCommandAttachmentImpl<eventID>::ptr_t)>>::value>::type* =        \
            nullptr)                                                                                                  \
    {                                                                                                                 \
        CBaseEventHandlersImpl<ECmdType>::registerHandler<_cmd>(_handler);                                            \
    }

namespace dds
{
    namespace protocol_api
    {

        class CChannelMessageHandlersImpl : private CBaseEventHandlersImpl<ECmdType>
        {

          public:
            template <class... Args>
            void dispatchHandlers(ECmdType _cmd, Args&&... args)
            {
                CBaseEventHandlersImpl<ECmdType>::dispatchHandlers<>(_cmd, std::forward<Args>(args)...);
            }

          public:
            bool handlerExists(ECmdType _cmd) const
            {
                return CBaseEventHandlersImpl<ECmdType>::handlerExists(_cmd);
            }

            REGISTER_EVENT_HANDLER(cmdREPLY_HANDSHAKE_OK)
            REGISTER_EVENT_HANDLER(cmdREPLY_HANDSHAKE_ERR)
            REGISTER_EVENT_HANDLER(cmdSHUTDOWN)
            REGISTER_EVENT_HANDLER(cmdUPDATE_KEY)
            REGISTER_EVENT_HANDLER(cmdUPDATE_KEY_ERROR)
            REGISTER_EVENT_HANDLER(cmdDELETE_KEY)
            REGISTER_EVENT_HANDLER(cmdCUSTOM_CMD)
            REGISTER_EVENT_HANDLER(cmdSIMPLE_MSG)
            REGISTER_EVENT_HANDLER(cmdHANDSHAKE)
            REGISTER_EVENT_HANDLER(cmdGET_LOG)
            REGISTER_EVENT_HANDLER(cmdBINARY_ATTACHMENT_RECEIVED)
            REGISTER_EVENT_HANDLER(cmdGET_AGENTS_INFO)
            REGISTER_EVENT_HANDLER(cmdSUBMIT)
            REGISTER_EVENT_HANDLER(cmdTRANSPORT_TEST)
            REGISTER_EVENT_HANDLER(cmdUSER_TASK_DONE)
            REGISTER_EVENT_HANDLER(cmdGET_PROP_LIST)
            REGISTER_EVENT_HANDLER(cmdGET_PROP_VALUES)
            REGISTER_EVENT_HANDLER(cmdUPDATE_TOPOLOGY)
            REGISTER_EVENT_HANDLER(cmdREPLY_ID)
            REGISTER_EVENT_HANDLER(cmdENABLE_STAT)
            REGISTER_EVENT_HANDLER(cmdGET_STAT)
            REGISTER_EVENT_HANDLER(cmdDISABLE_STAT)
            REGISTER_EVENT_HANDLER(cmdSTOP_USER_TASK)
        };
    }
}

#endif
