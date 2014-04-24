// Copyright 2014 GSI, Inc. All rights reserved.
//
// TODO: Describe
//
#ifndef DDSUSERDEFAULTSOPTIONS_H_
#define DDSUSERDEFAULTSOPTIONS_H_
// STD
#include <fstream>
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
// MiscCommon
#include "FindCfgFile.h"

namespace DDS
{
    class CDDSUserDefaults;

    inline std::string showCurrentPUDFile()
    {
        MiscCommon::CFindCfgFile<std::string> cfg;
        cfg.SetOrder("$HOME/.DDS/DDS.cfg")("$DDS_LOCATION/etc/DDS.cfg")("$DDS_LOCATION/DDS.cfg");
        std::string val;
        cfg.GetCfg(&val);

        return val;
    }
}

#endif /* DDSUSERDEFAULTSOPTIONS_H_ */
