// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__InfoChannel__
#define __DDS__InfoChannel__
// DDS
#include "ClientChannelImpl.h"
#include "GetPropValuesCmd.h"
#include "Options.h"

namespace dds
{
    namespace info_cmd
    {
        class CInfoChannel : public protocol_api::CClientChannelImpl<CInfoChannel>
        {
            CInfoChannel(boost::asio::io_service& _service)
                : CClientChannelImpl<CInfoChannel>(_service, protocol_api::EChannelType::UI)
                , m_nCounter(0)
            {
                registerHandler<protocol_api::EChannelEvents::OnHandshakeOK>([this]() {
                    // ask the server what we wnated to ask :)
                    if (m_options.m_bNeedCommanderPid || m_options.m_bNeedDDSStatus)
                        pushMsg<protocol_api::cmdGED_PID>();
                    else if (m_options.m_bNeedAgentsNumber || m_options.m_bNeedAgentsList)
                        pushMsg<protocol_api::cmdGET_AGENTS_INFO>();
                    else if (m_options.m_bNeedPropList)
                        pushMsg<protocol_api::cmdGET_PROP_LIST>();
                    else if (m_options.m_bNeedPropValues)
                    {
                        protocol_api::SGetPropValuesCmd cmd;
                        cmd.m_sPropertyID = m_options.m_propertyID;
                        pushMsg<protocol_api::cmdGET_PROP_VALUES>(cmd);
                    }
                });

                registerHandler<protocol_api::EChannelEvents::OnConnected>(
                    []() { LOG(MiscCommon::info) << "Connected to the commander server"; });

                registerHandler<protocol_api::EChannelEvents::OnFailedToConnect>(
                    []() { LOG(MiscCommon::log_stderr) << "Failed to connect to commander server."; });
            }

            REGISTER_DEFAULT_REMOTE_ID_STRING

          public:
            BEGIN_MSG_MAP(CInfoChannel)
                MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
                MESSAGE_HANDLER(cmdREPLY_PID, on_cmdREPLY_PID)
                MESSAGE_HANDLER(cmdREPLY_AGENTS_INFO, on_cmdREPLY_AGENTS_INFO)
            END_MSG_MAP()

            void setOptions(const SOptions& _options)
            {
                m_options = _options;
            }

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment);
            bool on_cmdREPLY_PID(protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_PID>::ptr_t _attachment);
            bool on_cmdREPLY_AGENTS_INFO(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_AGENTS_INFO>::ptr_t _attachment);

          private:
            SOptions m_options;
            std::mutex m_mutexCounter;
            size_t m_nCounter;
        };
    }
}
#endif
