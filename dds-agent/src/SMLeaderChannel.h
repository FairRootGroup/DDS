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
        CSMLeaderChannel(boost::asio::io_service& _service,
                         const std::string& _inputName,
                         const std::string& _outputName,
                         uint64_t _protocolHeaderID,
                         protocol_api::EMQOpenType _inputOpenType,
                         protocol_api::EMQOpenType _outputOpenType);

      public:
        ~CSMLeaderChannel();

      public:
        BEGIN_SM_MSG_MAP(CSMLeaderChannel)
            SM_MESSAGE_HANDLER(protocol_api::cmdLOBBY_MEMBER_INFO, on_cmdLOBBY_MEMBER_INFO)
        END_SM_MSG_MAP()

      private:
        // Message Handlers
        bool on_cmdLOBBY_MEMBER_INFO(
            protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
            const protocol_api::SSenderInfo& _sender);
    };
}
#endif
