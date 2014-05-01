// Copyright 2014 GSI, Inc. All rights reserved.
//
// TODO: Describe
//
#include "UserDefaults.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
// MiscCommon
#include "FindCfgFile.h"

using namespace std;
using namespace DDS;

std::string CUserDefaults::currentUDFile() const
{
    MiscCommon::CFindCfgFile<std::string> cfg;
    cfg.SetOrder("$HOME/.DDS/DDS.cfg")("$DDS_LOCATION/etc/DDS.cfg")("$DDS_LOCATION/DDS.cfg");
    std::string val;
    cfg.GetCfg(&val);

    return val;
}