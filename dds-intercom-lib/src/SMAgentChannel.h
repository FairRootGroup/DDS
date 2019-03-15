// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CSMAgentChannel__
#define __DDS__CSMAgentChannel__
// DDS
#include "BaseSMChannelImpl.h"

namespace dds
{
    namespace internal_api
    {
        class CSMAgentChannel : public protocol_api::CBaseSMChannelImpl<CSMAgentChannel>
        {
          protected:
            CSMAgentChannel(boost::asio::io_service& _service,
                            const std::string& _inputName,
                            const std::string& _outputName,
                            uint64_t _ProtocolHeaderID,
                            protocol_api::EMQOpenType _inputOpenType,
                            protocol_api::EMQOpenType _outputOpenType);

          public:
            BEGIN_SM_MSG_MAP(CSMAgentChannel)
                SM_MESSAGE_HANDLER_DISPATCH(cmdCUSTOM_CMD)
                SM_MESSAGE_HANDLER_DISPATCH(cmdUPDATE_KEY)
                SM_MESSAGE_HANDLER_DISPATCH(cmdSIMPLE_MSG)
                SM_MESSAGE_HANDLER_DISPATCH(cmdUSER_TASK_DONE)
            END_SM_MSG_MAP()
        };
    } // namespace internal_api
} // namespace dds
#endif
