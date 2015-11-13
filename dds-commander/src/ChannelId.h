// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDSCHANNELID_H
#define DDSCHANNELID_H

#include <cstdint>

namespace dds
{
    namespace commander_cmd
    {
        class DDSChannelId
        {
          public:
            static uint64_t getChannelId();
        };
    }
}
#endif
