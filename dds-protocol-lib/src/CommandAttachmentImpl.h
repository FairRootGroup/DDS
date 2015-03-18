// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef COMMANDATTACHMENTIMPL_H
#define COMMANDATTACHMENTIMPL_H
// DDS
#include "ProtocolCommands.h"
#include "ProtocolMessage.h"
#include "AgentsInfoCmd.h"
#include "SimpleMsgCmd.h"
#include "UUIDCmd.h"
#include "AssignUserTaskCmd.h"
#include "BinaryAttachmentCmd.h"
#include "HostInfoCmd.h"
#include "SubmitCmd.h"
#include "VersionCmd.h"
#include "BinaryAttachmentReceivedCmd.h"
#include "BinaryAttachmentStartCmd.h"
#include "UpdateKeyCmd.h"
#include "ProgressCmd.h"
#include "UserTaskDoneCmd.h"
#include "DeleteKeyCmd.h"
#include "GetPropValuesCmd.h"
#include "SetTopologyCmd.h"

#define REGISTER_CMD_ATTACHMENT(_class, _cmd)                                           \
    template <>                                                                         \
    struct SCommandAttachmentImpl<_cmd>                                                 \
    {                                                                                   \
        typedef std::shared_ptr<_class> ptr_t;                                          \
                                                                                        \
        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)                \
        {                                                                               \
            ptr_t p = std::make_shared<_class>();                                       \
            p->convertFromData(_msg->bodyToContainer());                                \
            return p;                                                                   \
        }                                                                               \
                                                                                        \
        static CProtocolMessage::protocolMessagePtr_t encode(const _class& _attachment) \
        {                                                                               \
            MiscCommon::BYTEVector_t data;                                              \
            _attachment.convertToData(&data);                                           \
            return std::make_shared<CProtocolMessage>(_cmd, data);                      \
        }                                                                               \
    };

namespace dds
{
    //----------------------------------------------------------------------
    struct SEmptyCmd
    {
    };
    //----------------------------------------------------------------------
    template <ECmdType>
    struct SCommandAttachmentImpl;
    //----------------------------------------------------------------------
    template <ECmdType _cmd>
    struct SCommandAttachmentImpl
    {
        typedef std::shared_ptr<SEmptyCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            return ptr_t();
        }

        static CProtocolMessage::protocolMessagePtr_t encode(const SEmptyCmd& /*_attachment*/)
        {
            MiscCommon::BYTEVector_t data;
            return std::make_shared<CProtocolMessage>(_cmd, data);
        }
    };

    REGISTER_CMD_ATTACHMENT(SVersionCmd, cmdHANDSHAKE)
    REGISTER_CMD_ATTACHMENT(SSimpleMsgCmd, cmdREPLY_HANDSHAKE_ERR)
    REGISTER_CMD_ATTACHMENT(SSubmitCmd, cmdSUBMIT)
    REGISTER_CMD_ATTACHMENT(SSimpleMsgCmd, cmdSIMPLE_MSG)
    REGISTER_CMD_ATTACHMENT(SHostInfoCmd, cmdREPLY_HOST_INFO)
    REGISTER_CMD_ATTACHMENT(SSimpleMsgCmd, cmdREPLY_PID)
    REGISTER_CMD_ATTACHMENT(SBinaryAttachmentCmd, cmdBINARY_ATTACHMENT)
    REGISTER_CMD_ATTACHMENT(SUUIDCmd, cmdREPLY_UUID)
    REGISTER_CMD_ATTACHMENT(SUUIDCmd, cmdSET_UUID)
    REGISTER_CMD_ATTACHMENT(SAgentsInfoCmd, cmdREPLY_AGENTS_INFO)
    REGISTER_CMD_ATTACHMENT(SAssignUserTaskCmd, cmdASSIGN_USER_TASK)
    REGISTER_CMD_ATTACHMENT(SBinaryAttachmentReceivedCmd, cmdBINARY_ATTACHMENT_RECEIVED)
    REGISTER_CMD_ATTACHMENT(SBinaryAttachmentStartCmd, cmdBINARY_ATTACHMENT_START)
    REGISTER_CMD_ATTACHMENT(SUpdateKeyCmd, cmdUPDATE_KEY)
    REGISTER_CMD_ATTACHMENT(SProgressCmd, cmdPROGRESS)
    REGISTER_CMD_ATTACHMENT(SUserTaskDoneCmd, cmdUSER_TASK_DONE)
    REGISTER_CMD_ATTACHMENT(SDeleteKeyCmd, cmdDELETE_KEY)
    REGISTER_CMD_ATTACHMENT(SGetPropValuesCmd, cmdGET_PROP_VALUES)
    REGISTER_CMD_ATTACHMENT(SSetTopologyCmd, cmdSET_TOPOLOGY)
}

#endif /* PROTOCOLMESSAGES_H_ */
