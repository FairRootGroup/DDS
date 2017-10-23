// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CSMFWChannel__
#define __DDS__CSMFWChannel__
// DDS
#include "BaseSMChannelImpl.h"

namespace dds
{
    class CSMFWChannel : public protocol_api::CBaseSMChannelImpl<CSMFWChannel>
    {
      protected:
        CSMFWChannel(const std::string& _inputName, const std::string& _outputName, uint64_t _ProtocolHeaderID);

      public:
        ~CSMFWChannel();

      public:
        SM_RAW_MESSAGE_HANDLER(CSMFWChannel, on_rawMessage)

      private:
        bool on_rawMessage(protocol_api::CProtocolMessage::protocolMessagePtr_t _currentMsg,
                           const protocol_api::SSenderInfo& _sender);
    };
}
#endif
