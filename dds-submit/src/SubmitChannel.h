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
    namespace submit_cmd
    {
        class CSubmitChannel : public CClientChannelImpl<CSubmitChannel>
        {
            CSubmitChannel(boost::asio::io_service& _service);

            REGISTER_DEFAULT_REMOTE_ID_STRING

          public:
            BEGIN_MSG_MAP(CSubmitChannel)
            MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
            MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
            END_MSG_MAP()

          public:
            void setSSHCfgFile(const std::string& _val);
            void setRMSTypeCode(const SSubmitCmd::ERmsType& _val);

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
            bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);

          private:
            std::string m_sSSHCfgFile;
            SSubmitCmd::ERmsType m_RMS;
            bool m_bXMLValidationDisabled;
        };
    }
}

#endif
