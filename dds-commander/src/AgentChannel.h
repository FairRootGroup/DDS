// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__CAgentChannel__
#define __DDS__CAgentChannel__
// DDS
#include "ConnectionImpl.h"

namespace dds
{
    enum class EAgentChannelType
    {
        UNDEFINED,
        AGENT,
        UI
    };

    class CAgentChannel : public CConnectionImpl<CAgentChannel>
    {
        CAgentChannel(boost::asio::io_service& _service)
            : CConnectionImpl<CAgentChannel>(_service)
            , m_isHandShakeOK(false)
            , m_type(EAgentChannelType::UNDEFINED)
        {
        }

        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CAgentChannel)
        MESSAGE_HANDLER(cmdHANDSHAKE, on_cmdHANDSHAKE)
        MESSAGE_HANDLER(cmdHANDSHAKE_AGENT, on_cmdHANDSHAKE_AGENT)
        MESSAGE_HANDLER(cmdREPLY_HOST_INFO, on_cmdREPLY_HOST_INFO)
        //====> replay on the "submit" command request
        MESSAGE_HANDLER(cmdSUBMIT, on_cmdSUBMIT)
        MESSAGE_HANDLER(cmdSUBMIT_START, on_cmdSUBMIT_START)
        //====> replay on the "info" command request
        // - get pid of the commander server
        MESSAGE_HANDLER(cmdGED_PID, on_cmdGED_PID)
        // - get Agents Info command
        MESSAGE_HANDLER(cmdGET_AGENTS_INFO, on_cmdGET_AGENTS_INFO)

        MESSAGE_HANDLER(cmdBINARY_DOWNLOAD_STAT, on_cmdBINARY_DOWNLOAD_STAT)
        MESSAGE_HANDLER(cmdREPLY_UUID, on_cmdREPLY_UUID)
        MESSAGE_HANDLER(cmdGET_LOG, on_cmdGET_LOG)
        MESSAGE_HANDLER(cmdBINARY_ATTACHMENT_LOG, on_cmdBINARY_ATTACHMENT_LOG)
        MESSAGE_HANDLER(cmdGET_LOG_ERROR, on_cmdGET_LOG_ERROR)
        END_MSG_MAP()

      public:
        EAgentChannelType getType() const;
        const boost::uuids::uuid& getId() const;
        const SHostInfoCmd getRemoteHostInfo() const
        {
            return m_remoteHostInfo;
        }

      private:
        // Message Handlers
        bool on_cmdHANDSHAKE(const CProtocolMessage& _msg);
        bool on_cmdHANDSHAKE_AGENT(const CProtocolMessage& _msg);
        bool on_cmdSUBMIT(const CProtocolMessage& _msg);
        bool on_cmdSUBMIT_START(const CProtocolMessage& _msg);
        bool on_cmdREPLY_HOST_INFO(const CProtocolMessage& _msg);
        bool on_cmdGED_PID(const CProtocolMessage& _msg);
        bool on_cmdBINARY_DOWNLOAD_STAT(const CProtocolMessage& _msg);
        bool on_cmdREPLY_UUID(const CProtocolMessage& _msg);
        bool on_cmdGET_LOG(const CProtocolMessage& _msg);
        bool on_cmdBINARY_ATTACHMENT_LOG(const CProtocolMessage& _msg);
        bool on_cmdGET_AGENTS_INFO(const CProtocolMessage& _msg);
        bool on_cmdGET_LOG_ERROR(const CProtocolMessage& _msg);

        // On connection handles
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::info) << "The Agent has closed the connection.";
        }
        // On header read handle
        void onHeaderRead();

      private:
        bool m_isHandShakeOK;
        EAgentChannelType m_type;
        boost::uuids::uuid m_id;
        SHostInfoCmd m_remoteHostInfo;
    };
}
#endif /* defined(__DDS__CAgentChannel__) */
