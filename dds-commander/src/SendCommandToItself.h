// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__SendCommandToItself__
#define __DDS__SendCommandToItself__
// DDS
#include "ConnectionImpl.h"

namespace dds
{
    class CSendCommandToItself : public CConnectionImpl<CSendCommandToItself>
    {
        CSendCommandToItself(boost::asio::io_service& _service)
            : CConnectionImpl<CSendCommandToItself>(_service)
            , m_isHandShakeOK(false)
        {
        }

      public:
        BEGIN_MSG_MAP(CSendCommandToItself)
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

      private:
        bool m_isHandShakeOK;
        std::string m_sTopoFile;
    };
}

#endif
