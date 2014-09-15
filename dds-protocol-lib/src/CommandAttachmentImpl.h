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
#include "BinaryDownloadStatCmd.h"
#include "SimpleMsgCmd.h"
#include "UUIDCmd.h"
#include "AssignUserTaskCmd.h"
#include "BinaryAttachmentCmd.h"
#include "HostInfoCmd.h"
#include "SubmitCmd.h"
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
    REGISTER_CMD_ATTACHMENT(SVersionCmd, cmdHANDSHAKE_AGENT)
    REGISTER_CMD_ATTACHMENT(SSubmitCmd, cmdSUBMIT)
    REGISTER_CMD_ATTACHMENT(SSimpleMsgCmd, cmdSIMPLE_MSG)
    REGISTER_CMD_ATTACHMENT(SHostInfoCmd, cmdREPLY_HOST_INFO)
    REGISTER_CMD_ATTACHMENT(SSimpleMsgCmd, cmdREPLY_PID)
    REGISTER_CMD_ATTACHMENT(SBinaryAttachmentCmd, cmdBINARY_ATTACHMENT)
    REGISTER_CMD_ATTACHMENT(SBinaryDownloadStatCmd, cmdBINARY_DOWNLOAD_STAT)
    REGISTER_CMD_ATTACHMENT(SUUIDCmd, cmdREPLY_UUID)
    REGISTER_CMD_ATTACHMENT(SUUIDCmd, cmdSET_UUID)
    REGISTER_CMD_ATTACHMENT(SAgentsInfoCmd, cmdREPLY_AGENTS_INFO)
    REGISTER_CMD_ATTACHMENT(SAssignUserTaskCmd, cmdASSIGN_USER_TASK)
}

#endif /* PROTOCOLMESSAGES_H_ */
