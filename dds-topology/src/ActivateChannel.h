// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ActivateChannel__
#define __DDS__ActivateChannel__
// DDS
#include "ClientChannelImpl.h"
#include "Options.h"

namespace dds
{
    class CActivateChannel : public CClientChannelImpl<CActivateChannel>
    {
        CActivateChannel(boost::asio::io_service& _service)
            : CClientChannelImpl<CActivateChannel>(_service, EChannelType::UI)
        {
            subscribeOnEvent(EChannelEvents::OnRemoteEndDissconnected,
                             [](CActivateChannel* _channel)
                             {
                                 LOG(MiscCommon::log_stderr) << "Server has closed the connection.";
                             });

            subscribeOnEvent(EChannelEvents::OnConnected,
                             [this](CActivateChannel* _channel)
                             {
                                 LOG(MiscCommon::log_stdout) << "Connection established.";
                                 switch (m_options.m_topologyCmd)
                                 {
                                     case ETopologyCmdType::ACTIVATE:
                                         LOG(MiscCommon::log_stdout) << "Requesting server to activate user tasks...";
                                         break;
                                     case ETopologyCmdType::STOP:
                                         LOG(MiscCommon::log_stdout) << "Requesting server to stop user tasks...";
                                         break;
                                     default:
                                         return;
                                 }
                             });

            subscribeOnEvent(EChannelEvents::OnFailedToConnect,
                             [this](CActivateChannel* _channel)
                             {
                                 LOG(MiscCommon::log_stdout) << "Failed to connect.";
                             });

            subscribeOnEvent(EChannelEvents::OnHandshakeOK,
                             [this](CActivateChannel* _channel)
                             {
                                 switch (m_options.m_topologyCmd)
                                 {
                                     case ETopologyCmdType::ACTIVATE:
                                         pushMsg<cmdACTIVATE_AGENT>();
                                         break;
                                     case ETopologyCmdType::STOP:
                                         pushMsg<cmdSTOP_USER_TASK>();
                                         break;
                                     default:
                                         return;
                                 }
                             });
        }

        REGISTER_DEFAULT_REMOTE_ID_STRING

      public:
        BEGIN_MSG_MAP(CActivateChannel)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        MESSAGE_HANDLER(cmdPROGRESS, on_cmdPROGRESS)
        END_MSG_MAP()

        void setOptions(const dds::SOptions& _options)
        {
            m_options = _options;
        }

      private:
        // Message Handlers
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
        bool on_cmdPROGRESS(SCommandAttachmentImpl<cmdPROGRESS>::ptr_t _attachment);

      private:
        dds::SOptions m_options;
    };
}

#endif
