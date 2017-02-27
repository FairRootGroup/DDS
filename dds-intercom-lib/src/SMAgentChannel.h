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
            CSMAgentChannel(const std::string& _inputName, const std::string& _outputName)
                : CBaseSMChannelImpl<CSMAgentChannel>(_inputName, _outputName)
            {
            }

          public:
            BEGIN_SM_MSG_MAP(CSMAgentChannel)
                SM_MESSAGE_HANDLER(cmdCUSTOM_CMD, on_cmdCUSTOM_CMD)
                SM_MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
                SM_MESSAGE_HANDLER(cmdUPDATE_KEY_ERROR, on_cmdUPDATE_KEY_ERROR)
                SM_MESSAGE_HANDLER(cmdDELETE_KEY, on_cmdDELETE_KEY)
                SM_MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
            END_SM_MSG_MAP()

          private:
            bool on_cmdCUSTOM_CMD(protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment);
            bool on_cmdUPDATE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
            bool on_cmdUPDATE_KEY_ERROR(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY_ERROR>::ptr_t _attachment);
            bool on_cmdDELETE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdDELETE_KEY>::ptr_t _attachment);
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment);
        };
    }
}
#endif
