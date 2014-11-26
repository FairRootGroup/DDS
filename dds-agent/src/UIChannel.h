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
        std::string _remoteEndIDString();

        REGISTER_DEFAULT_ON_CONNECT_CALLBACKS
        REGISTER_DEFAULT_ON_DISCONNECT_CALLBACKS

      public:
        BEGIN_MSG_MAP(CUIChannel)
        MESSAGE_HANDLER(cmdHANDSHAKE_KEY_VALUE_GUARD, on_cmdHANDSHAKE_KEY_VALUE_GUARD)
        MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
        MESSAGE_HANDLER(cmdWAIT_FOR_KEY_UPDATE, on_cmdWAIT_FOR_KEY_UPDATE)
        END_MSG_MAP()

      public:
        std::string getTypeName() const;
        bool isWaitingForKey() const;

      private:
        // Message Handlers
        bool on_cmdHANDSHAKE_KEY_VALUE_GUARD(SCommandAttachmentImpl<cmdHANDSHAKE_KEY_VALUE_GUARD>::ptr_t _attachment);
        bool on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment);
        bool on_cmdWAIT_FOR_KEY_UPDATE(SCommandAttachmentImpl<cmdWAIT_FOR_KEY_UPDATE>::ptr_t _attachment);

      private:
        bool m_isHandShakeOK;
        EChannelType m_type;
        bool m_isWaitingForKeyUpdates;
    };
}

#endif
