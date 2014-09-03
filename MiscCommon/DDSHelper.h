// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_DDSHELPER_H
#define DDS_DDSHELPER_H

// DDS
#include "UserDefaults.h"
#include "Logger.h"

// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace MiscCommon
{
    inline void findCommanderServer(std::string* _host, std::string* _port)
    {
        if (nullptr == _host || nullptr == _port)
            throw std::invalid_argument("findCommanderServer: Arguments must not be null");

        // Read server info file
        const std::string sSrvCfg(dds::CUserDefaults::instance().getServerInfoFileLocationSrv());
        LOG(MiscCommon::info) << "Reading server info from: " << sSrvCfg;
        if (sSrvCfg.empty())
            throw std::runtime_error("Can't find server info file.");

        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(sSrvCfg, pt);
        *_host = pt.get<std::string>("server.host");
        *_port = pt.get<std::string>("server.port");
    }
};

#endif
