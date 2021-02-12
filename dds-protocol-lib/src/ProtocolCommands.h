// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef PROTOCOLCOMMANDS_H_
#define PROTOCOLCOMMANDS_H_
// MiscCommon
#include "def.h"

#define NAME_TO_STRING(NAME) #NAME

// define current protocol version version
//
// We wanted to track protocol changes by versions, but at the moment it is a kinda overkill.
// At the moment it doesn't matter which versions peers have, once the version is different, we break the connection.
// In the future we might want to support backward compatibility. In this case protocol version, command will be
// organized in separate structures and enums.
//
const uint16_t g_protocolCommandsVersion = 5;

namespace dds
{
    namespace protocol_api
    {
        enum ECmdType
        {

            cmdUNKNOWN = 1,
            cmdRAW_MSG = 2, // special case - raw message processing
            cmdSHUTDOWN = 3,
            cmdHANDSHAKE,  // attachment: SVersionCmd
            cmdSUBMIT,     // attachment: SSubmitCmd
            cmdSIMPLE_MSG, // attachment: SSimpleMsgCmd
            cmdREPLY_HANDSHAKE_OK,
            cmdREPLY_HANDSHAKE_ERR, // attachment: SSimpleMsgCmd
            cmdGET_HOST_INFO,
            cmdREPLY_HOST_INFO, // attachment: SHostInfoCmd
            cmdGED_PID,
            cmdREPLY_PID,                  // attachment: SSimpleMsgCmd. The message contains the pid of the responder.
            cmdBINARY_ATTACHMENT,          // attachment: SBinanryAttachmentCmd. The message contains binary attachment.
            cmdBINARY_ATTACHMENT_RECEIVED, // attachment: SBinaryAttachmentReceivedCmd.
            cmdBINARY_ATTACHMENT_START,    // attachment: SBinaryAttachmentStartCmd.
            cmdGET_ID,
            cmdREPLY_ID, // attachment: SUUIDCmd
            cmdSET_ID,   // attachment: SUUIDCmd
            cmdGET_LOG,
            cmdGET_AGENTS_INFO,
            cmdREPLY_AGENTS_INFO, // attachment: SAgentsInfoCmd
            cmdASSIGN_USER_TASK,  // attachment: SAssignUserTaskCmd
            //  this command activates a given agent and triggers a start of an assigned user task
            cmdACTIVATE_USER_TASK, // attachment: SIDCmd
            cmdSTOP_USER_TASK,
            cmdUSER_TASK_DONE, // attachment: SUserTaskDoneCmd
            cmdTRANSPORT_TEST,
            cmdUPDATE_KEY, // attachment: SUpdateKeyCmd
            cmdGET_PROP_LIST,
            cmdGET_PROP_VALUES,
            cmdPROGRESS, // attachment: SProgressCmd
            cmdWATCHDOG_HEARTBEAT,
            cmdSET_TOPOLOGY,           // attachment: SSetTopologyCmd
            cmdUPDATE_TOPOLOGY,        // attachment: SUpdateTopologyCmd
            cmdCUSTOM_CMD,             // attachment: SCustomCmdCmd
            cmdLOBBY_MEMBER_INFO,      // attachment: SSimpleMsgCmd
            cmdLOBBY_MEMBER_HANDSHAKE, // attachment: SVersionCmd
            cmdREPLY,                  // attachment: SReplyCmd
            cmdGET_IDLE_AGENTS_COUNT,
            cmdREPLY_IDLE_AGENTS_COUNT, // attachment: SSimpleMsgCmd
            cmdADD_SLOT,                // attachment: SIDCmd
            cmdREPLY_ADD_SLOT           // attachment: SUUIDCmd
        };

        static std::map<uint16_t, std::string> g_cmdToString{
            { cmdUNKNOWN, NAME_TO_STRING(cmdUNKNOWN) },
            { cmdRAW_MSG, NAME_TO_STRING(cmdRAW_MSG) },
            { cmdSHUTDOWN, NAME_TO_STRING(cmdSHUTDOWN) },
            { cmdHANDSHAKE, NAME_TO_STRING(cmdHANDSHAKE) },
            { cmdSUBMIT, NAME_TO_STRING(cmdSUBMIT) },
            { cmdSIMPLE_MSG, NAME_TO_STRING(cmdSIMPLE_MSG) },
            { cmdREPLY_HANDSHAKE_OK, NAME_TO_STRING(cmdREPLY_HANDSHAKE_OK) },
            { cmdREPLY_HANDSHAKE_ERR, NAME_TO_STRING(cmdREPLY_HANDSHAKE_ERR) },
            { cmdGET_HOST_INFO, NAME_TO_STRING(cmdGET_HOST_INFO) },
            { cmdREPLY_HOST_INFO, NAME_TO_STRING(cmdREPLY_HOST_INFO) },
            { cmdGED_PID, NAME_TO_STRING(cmdGED_PID) },
            { cmdREPLY_PID, NAME_TO_STRING(cmdREPLY_PID) },
            { cmdBINARY_ATTACHMENT, NAME_TO_STRING(cmdBINARY_ATTACHMENT) },
            { cmdBINARY_ATTACHMENT_RECEIVED, NAME_TO_STRING(cmdBINARY_ATTACHMENT_RECEIVED) },
            { cmdBINARY_ATTACHMENT_START, NAME_TO_STRING(cmdBINARY_ATTACHMENT_START) },
            { cmdGET_ID, NAME_TO_STRING(cmdGET_ID) },
            { cmdREPLY_ID, NAME_TO_STRING(cmdREPLY_ID) },
            { cmdSET_ID, NAME_TO_STRING(cmdSET_ID) },
            { cmdGET_LOG, NAME_TO_STRING(cmdGET_LOG) },
            { cmdGET_AGENTS_INFO, NAME_TO_STRING(cmdGET_AGENTS_INFO) },
            { cmdREPLY_AGENTS_INFO, NAME_TO_STRING(cmdREPLY_AGENTS_INFO) },
            { cmdASSIGN_USER_TASK, NAME_TO_STRING(cmdASSIGN_USER_TASK) },
            { cmdACTIVATE_USER_TASK, NAME_TO_STRING(cmdACTIVATE_USER_TASK) },
            { cmdSTOP_USER_TASK, NAME_TO_STRING(cmdSTOP_USER_TASK) },
            { cmdUSER_TASK_DONE, NAME_TO_STRING(cmdUSER_TASK_DONE) },
            { cmdTRANSPORT_TEST, NAME_TO_STRING(cmdTRANSPORT_TEST) },
            { cmdUPDATE_KEY, NAME_TO_STRING(cmdUPDATE_KEY) },
            { cmdGET_PROP_LIST, NAME_TO_STRING(cmdGET_PROP_LIST) },
            { cmdGET_PROP_VALUES, NAME_TO_STRING(cmdGET_PROP_VALUES) },
            { cmdPROGRESS, NAME_TO_STRING(cmdPROGRESS) },
            { cmdWATCHDOG_HEARTBEAT, NAME_TO_STRING(cmdWATCHDOG_HEARTBEAT) },
            { cmdSET_TOPOLOGY, NAME_TO_STRING(cmdSET_TOPOLOGY) },
            { cmdUPDATE_TOPOLOGY, NAME_TO_STRING(cmdUPDATE_TOPOLOGY) },
            { cmdCUSTOM_CMD, NAME_TO_STRING(cmdCUSTOM_CMD) },
            { cmdLOBBY_MEMBER_INFO, NAME_TO_STRING(cmdLOBBY_MEMBER_INFO) },
            { cmdLOBBY_MEMBER_HANDSHAKE, NAME_TO_STRING(cmdLOBBY_MEMBER_HANDSHAKE) },
            { cmdREPLY, NAME_TO_STRING(cmdREPLY) },
            { cmdGET_IDLE_AGENTS_COUNT, NAME_TO_STRING(cmdGET_IDLE_AGENT_COUNT) },
            { cmdREPLY_IDLE_AGENTS_COUNT, NAME_TO_STRING(cmdREPLY_IDLE_AGENT_COUNT) },
            { cmdADD_SLOT, NAME_TO_STRING(cmdADD_SLOT) },
            { cmdREPLY_ADD_SLOT, NAME_TO_STRING(cmdREPLY_ADD_SLOT) }
        };
    } // namespace protocol_api
} // namespace dds

#endif /* PROTOCOLMESSAGES_H_ */
