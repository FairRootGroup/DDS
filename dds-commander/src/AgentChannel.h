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
        MESSAGE_HANDLER(cmdACTIVATE_AGENT, on_cmdACTIVATE_AGENT)
        //====> replay on the "info" command request
        // - get pid of the commander server
        MESSAGE_HANDLER(cmdGED_PID, on_cmdGED_PID)
        // - get Agents Info command
        MESSAGE_HANDLER(cmdGET_AGENTS_INFO, on_cmdGET_AGENTS_INFO)

        MESSAGE_HANDLER(cmdREPLY_UUID, on_cmdREPLY_UUID)
        MESSAGE_HANDLER(cmdGET_LOG, on_cmdGET_LOG)
        MESSAGE_HANDLER(cmdBINARY_ATTACHMENT_RECEIVED, on_cmdBINARY_ATTACHMENT_RECEIVED)
        MESSAGE_HANDLER(cmdTRANSPORT_TEST, on_cmdTRANSPORT_TEST)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
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
        bool on_cmdHANDSHAKE(SCommandAttachmentImpl<cmdHANDSHAKE>::ptr_t _attachment);
        bool on_cmdHANDSHAKE_AGENT(SCommandAttachmentImpl<cmdHANDSHAKE_AGENT>::ptr_t _attachment);
        bool on_cmdSUBMIT(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment);
        bool on_cmdACTIVATE_AGENT(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment);
        bool on_cmdREPLY_HOST_INFO(SCommandAttachmentImpl<cmdREPLY_HOST_INFO>::ptr_t _attachment);
        bool on_cmdGED_PID(SCommandAttachmentImpl<cmdGED_PID>::ptr_t _attachment);
        bool on_cmdREPLY_UUID(SCommandAttachmentImpl<cmdREPLY_UUID>::ptr_t _attachment);
        bool on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment);
        bool on_cmdBINARY_ATTACHMENT_RECEIVED(SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment);
        bool on_cmdGET_AGENTS_INFO(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment);
        bool on_cmdTRANSPORT_TEST(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment);
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);

        // On connection handles
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::info) << "The Agent has closed the connection.";
        }
        // On header read handle
        void onHeaderRead();
        std::string _remoteEndIDString()
        {
            if (EAgentChannelType::AGENT == m_type)
                return boost::uuids::to_string(m_id);
            else
                return "UI client";
        }

      private:
        bool m_isHandShakeOK;
        EAgentChannelType m_type;
        boost::uuids::uuid m_id;
        SHostInfoCmd m_remoteHostInfo;
        std::string m_sCurrentTopoFile;
    };
}
#endif /* defined(__DDS__CAgentChannel__) */
