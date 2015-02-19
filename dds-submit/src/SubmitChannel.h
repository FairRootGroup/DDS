// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__SubmitChannel__
#define __DDS__SubmitChannel__
// DDS
#include "ClientChannelImpl.h"

namespace dds
{
    class CSubmitChannel : public CClientChannelImpl<CSubmitChannel>
    {
        CSubmitChannel(boost::asio::io_service& _service)
            : CClientChannelImpl<CSubmitChannel>(_service, EChannelType::UI)
            , m_RMS(SSubmitCmd::UNKNOWN)
        {
            subscribeOnEvent(EChannelEvents::OnRemoteEndDissconnected,
                             [](CSubmitChannel* _channel)
                             {
                                 LOG(MiscCommon::log_stderr) << "Server has closed the connection.";
                             });

            subscribeOnEvent(EChannelEvents::OnHandshakeOK,
                             [this](CSubmitChannel* _channel)
                             {
                                 if (!m_sTopoFile.empty() && SSubmitCmd::UNKNOWN != m_RMS)
                                 {
                                     // Create the command's attachment
                                     SSubmitCmd cmd;
                                     cmd.m_sTopoFile = m_sTopoFile;
                                     cmd.m_nRMSTypeCode = m_RMS;
                                     cmd.m_sSSHCfgFile = m_sSSHCfgFile;
                                     cmd.m_bXMLValidationDisabled = m_bXMLValidationDisabled;
                                     pushMsg<cmdSUBMIT>(cmd);
                                 }
                             });

            subscribeOnEvent(EChannelEvents::OnConnected,
                             [](CSubmitChannel* _channel)
                             {
                                 LOG(MiscCommon::log_stdout) << "Connection established.";
                                 LOG(MiscCommon::log_stdout) << "Requesting server to process job submission...";
                             });

            subscribeOnEvent(EChannelEvents::OnFailedToConnect,
                             [](CSubmitChannel* _channel)
                             {
                                 LOG(MiscCommon::log_stdout) << "Failed to connect.";
                             });
        }

        REGISTER_DEFAULT_REMOTE_ID_STRING

      public:
        BEGIN_MSG_MAP(CSubmitChannel)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        END_MSG_MAP()

      public:
        void setTopoFile(const std::string& _val);
        void setSSHCfgFile(const std::string& _val);
        void setRMSTypeCode(const SSubmitCmd::ERmsType& _val);
        void setXMLValidationDisabled(bool _val);

      private:
        // Message Handlers
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);

      private:
        std::string m_sTopoFile;
        std::string m_sSSHCfgFile;
        SSubmitCmd::ERmsType m_RMS;
        bool m_bXMLValidationDisabled;
    };
}

#endif
