// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_OCTOPUS_TESTBIGCUSTOMCMD_H
#define DDS_OCTOPUS_TESTBIGCUSTOMCMD_H

#include "OctopusTestImpl.h"

namespace dds
{
    namespace dds_octopus
    {
        //
        // Send "big_cmd" -->
        // Receive "return" <---
        //
        class CTestBigCustomCmd : public COctopusTestImpl<CTestBigCustomCmd>
        {
          public:
            CTestBigCustomCmd(const SOptions_t& _options);

          public:
            BEGIN_OCTOPUS_MSG_MAP("Big Custom Command")
                OCTOPUS_MSG_HANDLER(onReturnCmd, SOctopusProtocol_Return)
            END_OCTOPUS_MSG_MAP

          public:
            void _init();
            void onReturnCmd(const SOctopusProtocol_Return& _return, uint64_t _senderId);

          private:
            uint32_t m_cmdCrc32;
            size_t m_nConfirmedCRC;
        };
    }
}
#endif
