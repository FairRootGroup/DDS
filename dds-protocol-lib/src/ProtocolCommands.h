// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef PROTOCOLCOMMANDS_H_
#define PROTOCOLCOMMANDS_H_
// MiscCommon
#include "def.h"
// DDS
#include "AgentsInfoCmd.h"
#include "BinaryDownloadStatCmd.h"
#include "SimpleMsgCmd.h"
#include "UUIDCmd.h"
#include "AssignUserTaskCmd.h"
#include "BinaryAttachmentCmd.h"
#include "HostInfoCmd.h"
#include "SubmitCmd.h"
#include "VersionCmd.h"

#define NAME_TO_STRING(NAME) #NAME

#define REG_CMD_WITH_ATTACHMENT(cmd, attachment_class)                                                     \
    template <typename A>                                                                                  \
    struct validate_command_attachment<A, cmd>                                                             \
    {                                                                                                      \
        void operator()()                                                                                  \
        {                                                                                                  \
            static_assert(std::is_same<attachment_class, A>::value,                                        \
                          "Bad attachment to the protocol command: " #cmd " requires " #attachment_class); \
        }                                                                                                  \
    };

// define current protocol version version
const uint16_t g_protocolCommandsVersion = 1;

namespace dds
{
    enum ECmdType
    {
        // ----------- VERSION 1 --------------------
        cmdUNKNOWN = -1,
        cmdSHUTDOWN = 1,
        cmdHANDSHAKE,       // attachment: SVersionCmd
        cmdHANDSHAKE_AGENT, // attachment: SVersionCmd
        cmdSUBMIT,          // attachment: SSubmitCmd
        cmdAGENT_ACTIVATE,
        cmdSIMPLE_MSG, // attachment: SSimpleMsgCmd
        cmdREPLY_HANDSHAKE_OK,
        cmdREPLY_ERR_BAD_PROTOCOL_VERSION,
        cmdGET_HOST_INFO,
        cmdREPLY_HOST_INFO, // attachment: SHostInfoCmd
        cmdDISCONNECT,
        cmdGED_PID,
        cmdREPLY_PID,            // attachment: SSimpleMsgCmd. The message contians the pid of the responder.
        cmdBINARY_ATTACHMENT,    // attachment: SBinanryAttachmentCmd. The message containes binary attachment.
        cmdBINARY_DOWNLOAD_STAT, // attachment: SBinaryDownloadStatCmd.
        cmdGET_UUID,
        cmdREPLY_UUID,            // attachment: SUUIDCmd
        cmdSET_UUID,              // attachment: SUUIDCmd
        cmdBINARY_ATTACHMENT_LOG, // attachment: SBinanryAttachmentCmd.
        cmdGET_LOG,
        cmdGET_AGENTS_INFO,
        cmdREPLY_AGENTS_INFO, // attachment: SAgentsInfoCmd
        cmdASSIGN_USER_TASK,  // attachment: SAssignUserTaskCmd
        cmdACTIVATE_AGENT,    // this command activates a given agent and triggers a start of an assgined user task
        cmdSTART_DOWNLOAD_TEST,
        cmdDOWNLOAD_TEST,      // attachment: SBinanryAttachmentCmd
        cmdDOWNLOAD_TEST_STAT, // attachment: SBinaryDownloadStatCmd

        // ----------- VERSION 2 --------------------
    };

    static std::map<uint16_t, std::string> g_cmdToString{
        { cmdUNKNOWN, NAME_TO_STRING(cmdUNKNOWN) },
        { cmdSHUTDOWN, NAME_TO_STRING(cmdSHUTDOWN) },
        { cmdHANDSHAKE, NAME_TO_STRING(cmdHANDSHAKE) },
        { cmdHANDSHAKE_AGENT, NAME_TO_STRING(cmdHANDSHAKE_AGENT) },
        { cmdSUBMIT, NAME_TO_STRING(cmdSUBMIT) },
        { cmdAGENT_ACTIVATE, NAME_TO_STRING(cmdAGENT_ACTIVATE) },
        { cmdSIMPLE_MSG, NAME_TO_STRING(cmdSIMPLE_MSG) },
        { cmdREPLY_HANDSHAKE_OK, NAME_TO_STRING(cmdREPLY_HANDSHAKE_OK) },
        { cmdREPLY_ERR_BAD_PROTOCOL_VERSION, NAME_TO_STRING(cmdREPLY_ERR_BAD_PROTOCOL_VERSION) },
        { cmdGET_HOST_INFO, NAME_TO_STRING(cmdGET_HOST_INFO) },
        { cmdREPLY_HOST_INFO, NAME_TO_STRING(cmdREPLY_HOST_INFO) },
        { cmdDISCONNECT, NAME_TO_STRING(cmdDISCONNECT) },
        { cmdGED_PID, NAME_TO_STRING(cmdGED_PID) },
        { cmdREPLY_PID, NAME_TO_STRING(cmdREPLY_PID) },
        { cmdBINARY_ATTACHMENT, NAME_TO_STRING(cmdBINARY_ATTACHMENT) },
        { cmdBINARY_DOWNLOAD_STAT, NAME_TO_STRING(cmdBINARY_DOWNLOAD_STAT) },
        { cmdGET_UUID, NAME_TO_STRING(cmdGET_UUID) },
        { cmdREPLY_UUID, NAME_TO_STRING(cmdREPLY_UUID) },
        { cmdSET_UUID, NAME_TO_STRING(cmdSET_UUID) },
        { cmdBINARY_ATTACHMENT_LOG, NAME_TO_STRING(cmdBINARY_ATTACHMENT_LOG) },
        { cmdGET_LOG, NAME_TO_STRING(cmdGET_LOG) },
        { cmdGET_AGENTS_INFO, NAME_TO_STRING(cmdGET_AGENTS_INFO) },
        { cmdREPLY_AGENTS_INFO, NAME_TO_STRING(cmdREPLY_AGENTS_INFO) },
        { cmdASSIGN_USER_TASK, NAME_TO_STRING(cmdASSIGN_USER_TASK) },
        { cmdACTIVATE_AGENT, NAME_TO_STRING(cmdACTIVATE_AGENT) },
        { cmdSTART_DOWNLOAD_TEST, NAME_TO_STRING(cmdSTART_DOWNLOAD_TEST) },
        { cmdDOWNLOAD_TEST, NAME_TO_STRING(cmdDOWNLOAD_TEST) },
        { cmdDOWNLOAD_TEST_STAT, NAME_TO_STRING(cmdDOWNLOAD_TEST_STAT) }
    };

    //----------------------------------------------------------------------
    template <typename A, ECmdType>
    struct validate_command_attachment;

    REG_CMD_WITH_ATTACHMENT(cmdHANDSHAKE, SVersionCmd);
    REG_CMD_WITH_ATTACHMENT(cmdHANDSHAKE_AGENT, SVersionCmd);
    REG_CMD_WITH_ATTACHMENT(cmdSUBMIT, SSubmitCmd);
    REG_CMD_WITH_ATTACHMENT(cmdSIMPLE_MSG, SSimpleMsgCmd);
    REG_CMD_WITH_ATTACHMENT(cmdREPLY_HOST_INFO, SHostInfoCmd);
    REG_CMD_WITH_ATTACHMENT(cmdREPLY_PID, SSimpleMsgCmd);
    REG_CMD_WITH_ATTACHMENT(cmdBINARY_ATTACHMENT, SBinaryAttachmentCmd);
    REG_CMD_WITH_ATTACHMENT(cmdBINARY_DOWNLOAD_STAT, SBinaryDownloadStatCmd);
    REG_CMD_WITH_ATTACHMENT(cmdBINARY_ATTACHMENT_LOG, SBinaryAttachmentCmd);
    REG_CMD_WITH_ATTACHMENT(cmdREPLY_UUID, SUUIDCmd);
    REG_CMD_WITH_ATTACHMENT(cmdSET_UUID, SUUIDCmd);
    REG_CMD_WITH_ATTACHMENT(cmdREPLY_AGENTS_INFO, SAgentsInfoCmd);
    REG_CMD_WITH_ATTACHMENT(cmdASSIGN_USER_TASK, SAssignUserTaskCmd);
    REG_CMD_WITH_ATTACHMENT(cmdDOWNLOAD_TEST_STAT, SBinaryDownloadStatCmd);
    REG_CMD_WITH_ATTACHMENT(cmdDOWNLOAD_TEST, SBinaryAttachmentCmd);
}

#endif /* PROTOCOLMESSAGES_H_ */
