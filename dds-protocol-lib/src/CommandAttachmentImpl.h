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

namespace dds
{
    //----------------------------------------------------------------------
    class SEmptyCmd
    {
    };
    //----------------------------------------------------------------------
    template <ECmdType>
    struct SCommandAttachmentImpl;
    //----------------------------------------------------------------------
    template <ECmdType>
    struct SCommandAttachmentImpl
    {
        typedef std::shared_ptr<SEmptyCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            return ptr_t();
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdHANDSHAKE>
    {
        typedef std::shared_ptr<SVersionCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SVersionCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdHANDSHAKE_AGENT>
    {
        typedef std::shared_ptr<SVersionCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SVersionCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdSUBMIT>
    {
        typedef std::shared_ptr<SSubmitCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SSubmitCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdSIMPLE_MSG>
    {
        typedef std::shared_ptr<SSimpleMsgCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SSimpleMsgCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdREPLY_HOST_INFO>
    {
        typedef std::shared_ptr<SHostInfoCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SHostInfoCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdREPLY_PID>
    {
        typedef std::shared_ptr<SSimpleMsgCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SSimpleMsgCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdBINARY_ATTACHMENT>
    {
        typedef std::shared_ptr<SBinaryAttachmentCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SBinaryAttachmentCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdBINARY_DOWNLOAD_STAT>
    {
        typedef std::shared_ptr<SBinaryDownloadStatCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SBinaryDownloadStatCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdREPLY_UUID>
    {
        typedef std::shared_ptr<SUUIDCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SUUIDCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdSET_UUID>
    {
        typedef std::shared_ptr<SUUIDCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SUUIDCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdREPLY_AGENTS_INFO>
    {
        typedef std::shared_ptr<SAgentsInfoCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SAgentsInfoCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
    //----------------------------------------------------------------------
    template <>
    struct SCommandAttachmentImpl<cmdASSIGN_USER_TASK>
    {
        typedef std::shared_ptr<SAssignUserTaskCmd> ptr_t;

        static ptr_t decode(CProtocolMessage::protocolMessagePtr_t _msg)
        {
            ptr_t p = std::make_shared<SAssignUserTaskCmd>();
            p->convertFromData(_msg->bodyToContainer());
            return p;
        }
    };
}

#endif /* PROTOCOLMESSAGES_H_ */
