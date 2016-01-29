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
        class CSubmitChannel : public protocol_api::CClientChannelImpl<CSubmitChannel>
        {
            CSubmitChannel(boost::asio::io_service& _service);

            REGISTER_DEFAULT_REMOTE_ID_STRING

          public:
            BEGIN_MSG_MAP(CSubmitChannel)
            MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
            MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
            END_MSG_MAP()

          public:
            void setCfgFile(const std::string& _val);
            void setRMSType(const std::string& _val);
            void setNumber(const size_t _val);

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment);
            bool on_cmdSHUTDOWN(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment);

          private:
            std::string m_sCfgFile;
            std::string m_sRMS;
            size_t m_number;
            bool m_bXMLValidationDisabled;
        };
    }
}

#endif
