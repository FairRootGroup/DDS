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
            void start(bool _onlyLocalSys);
            std::string getSessionID()
            {
                return m_sSessionID;
            }

          private:
            void getNewSessionID();
            /// returns false if there is a missing prec. binary.
            bool checkPrecompiledWNBins(bool _onlyLocalSys);
            void getPrecompiledWNBins(MiscCommon::StringVector_t& _list);
            void spawnDDSCommander();
            void checkCommanderStatus();
            void printHint();

          private:
            std::string m_sSessionID;
        };
    } // namespace session_cmd
} // namespace dds
#endif
