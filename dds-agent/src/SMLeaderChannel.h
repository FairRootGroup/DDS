// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CSMLeaderChannel__
#define __DDS__CSMLeaderChannel__
// DDS
#include "BaseSMChannelImpl.h"

namespace dds
{
    class CSMLeaderChannel : public protocol_api::CBaseSMChannelImpl<CSMLeaderChannel>
    {
      protected:
        CSMLeaderChannel(const std::string& _inputName, const std::string& _outputName, uint64_t _ProtocolHeaderID);

      public:
        ~CSMLeaderChannel();

      public:
        BEGIN_SM_MSG_MAP(CSMLeaderChannel)
            SM_MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
        END_SM_MSG_MAP()

      private:
        // Message Handlers
        bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                              protocol_api::SSenderInfo& _sender);
    };
}
#endif
