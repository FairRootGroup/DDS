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
    namespace topology_cmd
    {
        class CActivateChannel : public CClientChannelImpl<CActivateChannel>
        {
            CActivateChannel(boost::asio::io_service& _service);

            REGISTER_DEFAULT_REMOTE_ID_STRING

          public:
            BEGIN_MSG_MAP(CActivateChannel)
            MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
            MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
            MESSAGE_HANDLER(cmdPROGRESS, on_cmdPROGRESS)
            END_MSG_MAP()

            void setOptions(const SOptions& _options)
            {
                m_options = _options;
            }

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
            bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
            bool on_cmdPROGRESS(SCommandAttachmentImpl<cmdPROGRESS>::ptr_t _attachment);

          private:
            SOptions m_options;
        };
    }
}

#endif
