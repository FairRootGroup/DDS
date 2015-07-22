// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__InfoChannel__
#define __DDS__InfoChannel__
// DDS
#include "ClientChannelImpl.h"
#include "Options.h"
#include "GetPropValuesCmd.h"

namespace dds
{
    namespace info_cmd
    {
        class CInfoChannel : public CClientChannelImpl<CInfoChannel>
        {
            CInfoChannel(boost::asio::io_service& _service)
                : CClientChannelImpl<CInfoChannel>(_service, EChannelType::UI)
            {
                subscribeOnEvent(EChannelEvents::OnHandshakeOK,
                                 [this](CInfoChannel* _channel)
                                 {
                                     // ask the server what we wnated to ask :)
                                     if (m_options.m_bNeedCommanderPid || m_options.m_bNeedDDSStatus)
                                         pushMsg<cmdGED_PID>();
                                     else if (m_options.m_bNeedAgentsNumber || m_options.m_bNeedAgentsList)
                                         pushMsg<cmdGET_AGENTS_INFO>();
                                     else if (m_options.m_bNeedPropList)
                                         pushMsg<cmdGET_PROP_LIST>();
                                     else if (m_options.m_bNeedPropValues)
                                     {
                                         SGetPropValuesCmd cmd;
                                         cmd.m_sPropertyID = m_options.m_propertyID;
                                         pushMsg<cmdGET_PROP_VALUES>(cmd);
                                     }
                                 });

                subscribeOnEvent(EChannelEvents::OnConnected,
                                 [this](CInfoChannel* _channel)
                                 {
                                     LOG(MiscCommon::info) << "Connected to the commander server";
                                 });

                subscribeOnEvent(EChannelEvents::OnFailedToConnect,
                                 [this](CInfoChannel* _channel)
                                 {
                                     LOG(MiscCommon::log_stderr) << "Failed to connect to commander server.";
                                 });
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
            bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
            bool on_cmdREPLY_PID(SCommandAttachmentImpl<cmdREPLY_PID>::ptr_t _attachment);
            bool on_cmdREPLY_AGENTS_INFO(SCommandAttachmentImpl<cmdREPLY_AGENTS_INFO>::ptr_t _attachment);

          private:
            SOptions m_options;
        };
    }
}
#endif
