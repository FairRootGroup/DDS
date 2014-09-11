// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__SubmitChannel__
#define __DDS__SubmitChannel__
// DDS
#include "ConnectionImpl.h"

namespace dds
{
    class CSubmitChannel : public CConnectionImpl<CSubmitChannel>
    {
        CSubmitChannel(boost::asio::io_service& _service)
            : CConnectionImpl<CSubmitChannel>(_service)
            , m_isHandShakeOK(false)
            , m_RMS(SSubmitCmd::UNKNOWN)
            , m_bSendStart(false)
        {
        }

        REGISTER_DEFAULT_REMOTE_ID_STRING

      public:
        BEGIN_MSG_MAP(CSubmitChannel)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        END_MSG_MAP()

      public:
        void setTopoFile(const std::string& _val);
        void setSSHCfgFile(const std::string& _val);
        void setRMSTypeCode(const SSubmitCmd::ERmsType& _val);

        void setSendStart(bool _val = false)
        {
            m_bSendStart = _val;
        }

      private:
        // Message Handlers
        bool on_cmdREPLY_HANDSHAKE_OK(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment);
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
        // On connection handles
        void onConnected()
        {
            LOG(MiscCommon::log_stdout) << "Connection established.";
            LOG(MiscCommon::log_stdout) << "Requesting server to process job submission...";
        }
        void onFailedToConnect()
        {
            LOG(MiscCommon::log_stdout) << "Failed to connect.";
        }
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::log_stderr) << "Server has closed the coinnection.";
        }
        void onHeaderRead()
        {
        }

      private:
        bool m_isHandShakeOK;
        std::string m_sTopoFile;
        std::string m_sSSHCfgFile;
        SSubmitCmd::ERmsType m_RMS;
        bool m_bSendStart;
    };
}

#endif
