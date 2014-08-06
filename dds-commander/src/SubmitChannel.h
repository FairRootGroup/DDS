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
        {
        }

      public:
        BEGIN_MSG_MAP(CSubmitChannel)
        MESSAGE_HANDLER(cmdREPLY_HANDSHAKE_OK, on_cmdREPLY_HANDSHAKE_OK)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        MESSAGE_HANDLER(cmdREPLY_SUBMIT_OK, on_cmdREPLY_SUBMIT_OK)
        MESSAGE_HANDLER(cmdREPLY_ERR_SUBMIT, on_cmdREPLY_ERR_SUBMIT)
        END_MSG_MAP()

      public:
        void setTopoFile(const std::string& _topoFile);

      private:
        // Message Handlers
        int on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg);
        int on_cmdSIMPLE_MSG(const CProtocolMessage& _msg);
        int on_cmdREPLY_SUBMIT_OK(const CProtocolMessage& _msg);
        int on_cmdREPLY_ERR_SUBMIT(const CProtocolMessage& _msg);
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
    };
}

#endif
