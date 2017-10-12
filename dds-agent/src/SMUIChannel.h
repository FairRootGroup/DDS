// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CSMUIChannel__
#define __DDS__CSMUIChannel__
// DDS
#include "BaseSMChannelImpl.h"

namespace dds
{
    class CSMUIChannel : public protocol_api::CBaseSMChannelImpl<CSMUIChannel>
    {
      protected:
        CSMUIChannel(const std::string& _inputName, const std::string& _outputName);

      public:
        ~CSMUIChannel();

      public:
        BEGIN_SM_MSG_MAP(CSMUIChannel)
            SM_MESSAGE_HANDLER(cmdCUSTOM_CMD, on_cmdCUSTOM_CMD)
            SM_MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
        END_SM_MSG_MAP()

      private:
        bool on_cmdCUSTOM_CMD(protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment);
        bool on_cmdUPDATE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
    };
}
#endif
