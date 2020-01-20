// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CSMIntercomChannel__
#define __DDS__CSMIntercomChannel__
// DDS
#include "BaseSMChannelImpl.h"

namespace dds
{
    class CSMIntercomChannel : public protocol_api::CBaseSMChannelImpl<CSMIntercomChannel>
    {
      protected:
        CSMIntercomChannel(boost::asio::io_context& _service,
                           const std::vector<std::string>& _inputNames,
                           const std::string& _outputName,
                           uint64_t _protocolHeaderID,
                           protocol_api::EMQOpenType _inputOpenType,
                           protocol_api::EMQOpenType _outputOpenType);

      public:
        ~CSMIntercomChannel();

      public:
        BEGIN_SM_MSG_MAP(CSMIntercomChannel)
            SM_MESSAGE_HANDLER_DISPATCH(cmdCUSTOM_CMD)
            SM_MESSAGE_HANDLER_DISPATCH(cmdUPDATE_KEY)
            SM_MESSAGE_HANDLER_DISPATCH(cmdSIMPLE_MSG)
            SM_MESSAGE_HANDLER_DISPATCH(cmdUSER_TASK_DONE)
        END_SM_MSG_MAP()
    };
} // namespace dds
#endif
