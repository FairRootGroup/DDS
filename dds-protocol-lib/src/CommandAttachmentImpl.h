// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef COMMANDATTACHMENTIMPL_H
#define COMMANDATTACHMENTIMPL_H
// DDS
#include "AgentsInfoCmd.h"
#include "AssignUserTaskCmd.h"
#include "BinaryAttachmentCmd.h"
#include "BinaryAttachmentReceivedCmd.h"
#include "BinaryAttachmentStartCmd.h"
#include "CustomCmdCmd.h"
#include "DeleteKeyCmd.h"
#include "GetPropValuesCmd.h"
#include "HostInfoCmd.h"
#include "ProgressCmd.h"
#include "ProtocolCommands.h"
#include "ProtocolMessage.h"
#include "SetTopologyCmd.h"
#include "SimpleMsgCmd.h"
#include "SubmitCmd.h"
#include "UUIDCmd.h"
#include "UpdateKeyCmd.h"
#include "UpdateKeyErrorCmd.h"
#include "UpdateTopologyCmd.h"
#include "UserTaskDoneCmd.h"
#include "VersionCmd.h"

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
    namespace protocol_api
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
        REGISTER_CMD_ATTACHMENT(SIDCmd, cmdREPLY_ID)
        REGISTER_CMD_ATTACHMENT(SIDCmd, cmdSET_ID)
        REGISTER_CMD_ATTACHMENT(SAgentsInfoCmd, cmdREPLY_AGENTS_INFO)
        REGISTER_CMD_ATTACHMENT(SAssignUserTaskCmd, cmdASSIGN_USER_TASK)
        REGISTER_CMD_ATTACHMENT(SBinaryAttachmentReceivedCmd, cmdBINARY_ATTACHMENT_RECEIVED)
        REGISTER_CMD_ATTACHMENT(SBinaryAttachmentStartCmd, cmdBINARY_ATTACHMENT_START)
        REGISTER_CMD_ATTACHMENT(SUpdateKeyCmd, cmdUPDATE_KEY)
        REGISTER_CMD_ATTACHMENT(SUpdateKeyErrorCmd, cmdUPDATE_KEY_ERROR)
        REGISTER_CMD_ATTACHMENT(SProgressCmd, cmdPROGRESS)
        REGISTER_CMD_ATTACHMENT(SUserTaskDoneCmd, cmdUSER_TASK_DONE)
        REGISTER_CMD_ATTACHMENT(SDeleteKeyCmd, cmdDELETE_KEY)
        REGISTER_CMD_ATTACHMENT(SGetPropValuesCmd, cmdGET_PROP_VALUES)
        REGISTER_CMD_ATTACHMENT(SSetTopologyCmd, cmdSET_TOPOLOGY)
        REGISTER_CMD_ATTACHMENT(SUpdateTopologyCmd, cmdUPDATE_TOPOLOGY)
        REGISTER_CMD_ATTACHMENT(SCustomCmdCmd, cmdCUSTOM_CMD)
    }
}

#endif /* PROTOCOLMESSAGES_H_ */
