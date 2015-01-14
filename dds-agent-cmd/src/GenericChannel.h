// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__GenericChannel__
#define __DDS__GenericChannel__
// DDS
#include "ClientChannelImpl.h"
#include "Options.h"

namespace dds
{
    class CGenericChannel : public CClientChannelImpl<CGenericChannel>
    {
        CGenericChannel(boost::asio::io_service& _service)
            : CClientChannelImpl<CGenericChannel>(_service, EChannelType::UI)
        {
        }

        REGISTER_DEFAULT_REMOTE_ID_STRING
        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CGenericChannel)
        MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG);
        MESSAGE_HANDLER(cmdSHUTDOWN, on_cmdSHUTDOWN)
        END_MSG_MAP()

        void setOptions(const SOptions& _options)
        {
            m_options = _options;
        }

      private:
        // Message Handlers
        bool on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment);
        bool on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment);
        // On connection handles
        void onRemoteEndDissconnected()
        {
            LOG(MiscCommon::info) << "The DDS commander [" << socket().remote_endpoint().address().to_string()
                                  << "] has closed the connection.";
        }

        void onHandshakeOK();
        void onHandshakeERR();

      private:
        SOptions m_options;
    };
}
#endif /* defined(__DDS__GenericChannel__) */