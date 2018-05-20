// Copyright 2018 GSI, Inc. All rights reserved.
//
//
//
#ifndef STOP_H
#define STOP_H
// DDS
#include "def.h"
// STD
#include <string>

namespace dds
{
    namespace session_cmd
    {
        class CStop
        {
          public:
            void stop(const std::string& _sessionID);
        };
    } // namespace session_cmd
} // namespace dds
#endif
