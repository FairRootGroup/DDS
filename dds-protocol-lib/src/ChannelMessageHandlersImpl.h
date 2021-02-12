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

#define DDS_REGISTER_MESSAGE_HANDLER(eventID) \
    DDS_REGISTER_EVENT_HANDLER(ECmdType, eventID, void(const SSenderInfo&, SCommandAttachmentImpl<eventID>::ptr_t))

namespace dds
{
    namespace protocol_api
    {

        class CChannelMessageHandlersImpl : private CBaseEventHandlersImpl<ECmdType>
        {
            DDS_BEGIN_EVENT_HANDLERS(ECmdType)
            DDS_REGISTER_EVENT_HANDLER(ECmdType,
                                       cmdRAW_MSG,
                                       void(const protocol_api::SSenderInfo&,
                                            protocol_api::CProtocolMessage::protocolMessagePtr_t))
            DDS_REGISTER_MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK)
            DDS_REGISTER_MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_ERR)
            DDS_REGISTER_MESSAGE_HANDLER(cmdSHUTDOWN)
            DDS_REGISTER_MESSAGE_HANDLER(cmdUPDATE_KEY)
            DDS_REGISTER_MESSAGE_HANDLER(cmdCUSTOM_CMD)
            DDS_REGISTER_MESSAGE_HANDLER(cmdSIMPLE_MSG)
            DDS_REGISTER_MESSAGE_HANDLER(cmdHANDSHAKE)
            DDS_REGISTER_MESSAGE_HANDLER(cmdGET_LOG)
            DDS_REGISTER_MESSAGE_HANDLER(cmdBINARY_ATTACHMENT_RECEIVED)
            DDS_REGISTER_MESSAGE_HANDLER(cmdGET_AGENTS_INFO)
            DDS_REGISTER_MESSAGE_HANDLER(cmdGET_IDLE_AGENTS_COUNT)
            DDS_REGISTER_MESSAGE_HANDLER(cmdSUBMIT)
            DDS_REGISTER_MESSAGE_HANDLER(cmdTRANSPORT_TEST)
            DDS_REGISTER_MESSAGE_HANDLER(cmdUSER_TASK_DONE)
            DDS_REGISTER_MESSAGE_HANDLER(cmdGET_PROP_LIST)
            DDS_REGISTER_MESSAGE_HANDLER(cmdGET_PROP_VALUES)
            DDS_REGISTER_MESSAGE_HANDLER(cmdUPDATE_TOPOLOGY)
            DDS_REGISTER_MESSAGE_HANDLER(cmdREPLY_ID)
            DDS_REGISTER_MESSAGE_HANDLER(cmdSTOP_USER_TASK)
            DDS_REGISTER_MESSAGE_HANDLER(cmdLOBBY_MEMBER_HANDSHAKE)
            DDS_REGISTER_MESSAGE_HANDLER(cmdREPLY)
            DDS_END_EVENT_HANDLERS
        };
    } // namespace protocol_api
} // namespace dds

#endif
