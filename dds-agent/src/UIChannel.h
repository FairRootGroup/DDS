// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_UIChannel_h
#define DDS_UIChannel_h

// DDS
#include "ConnectionImpl.h"

namespace dds
{
    enum class EChannelType
    {
        UNDEFINED,
        KEY_VALUE_GUARD
    };
    const std::vector<std::string> g_vecChannelType = { "generic", "key-value-guard" };

    class CUIChannel : public CConnectionImpl<CUIChannel>
    {
      private:
        CUIChannel(boost::asio::io_service& _service);

        REGISTER_DEFAULT_REMOTE_ID_STRING
        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS
        REGISTER_DEFAULT_ON_DISCONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CUIChannel)
        MESSAGE_HANDLER(cmdHANDSHAKE_KEY_VALUE_GUARD, on_cmdHANDSHAKE_KEY_VALUE_GUARD)
        END_MSG_MAP()

      public:
        std::string getTypeName() const;

      private:
        // Message Handlers
        bool on_cmdHANDSHAKE_KEY_VALUE_GUARD(SCommandAttachmentImpl<cmdHANDSHAKE_KEY_VALUE_GUARD>::ptr_t _attachment);

      private:
        bool m_isHandShakeOK;
        EChannelType m_type;
    };
}

#endif
