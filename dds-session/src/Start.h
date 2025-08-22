// Copyright 2018 GSI, Inc. All rights reserved.
//
//
//
#ifndef START_H
#define START_H
// DDS
#include "def.h"
// STD
#include <string>

namespace dds
{
    namespace session_cmd
    {
        class CStart
        {
          public:
            void start(bool _Mixed, bool _Lightweight = false);
            std::string getSessionID()
            {
                return m_sSessionID;
            }

          private:
            void getNewSessionID();
            /// returns false if there is a missing prec. binary.
            bool checkPrecompiledWNBins(bool _Mixed, bool _Lightweight);
            void getPrecompiledWNBins(dds::misc::StringVector_t& _list);
            void spawnDDSCommander();
            void checkCommanderStatus();
            void printHint();

          private:
            std::string m_sSessionID;
        };
    } // namespace session_cmd
} // namespace dds
#endif
