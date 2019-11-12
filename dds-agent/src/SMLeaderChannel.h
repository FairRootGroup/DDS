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
        CSMLeaderChannel(boost::asio::io_context& _service,
                         const std::vector<std::string>& _inputNames,
                         const std::string& _outputName,
                         uint64_t _protocolHeaderID,
                         protocol_api::EMQOpenType _inputOpenType,
                         protocol_api::EMQOpenType _outputOpenType);

      public:
        ~CSMLeaderChannel();

      public:
        BEGIN_SM_MSG_MAP(CSMLeaderChannel)
            SM_MESSAGE_HANDLER_DISPATCH(cmdCUSTOM_CMD)
            SM_MESSAGE_HANDLER_DISPATCH(cmdUPDATE_KEY)
            SM_MESSAGE_HANDLER_DISPATCH(cmdSIMPLE_MSG)
            SM_MESSAGE_HANDLER_DISPATCH(cmdUSER_TASK_DONE)
        END_SM_MSG_MAP()
    };
} // namespace dds
#endif
