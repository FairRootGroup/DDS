// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_DDSHELPER_H
#define DDS_DDSHELPER_H

// DDS
#include "Logger.h"
#include "UserDefaults.h"

// BOOST
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace MiscCommon
{
    inline void findCommanderServerImpl(std::string* _host, std::string* _port, const std::string& _name)
    {
        if (nullptr == _host || nullptr == _port)
            throw std::invalid_argument("findCommanderServer: Arguments must not be null");

        // Read server info file
        const std::string sSrvCfg(dds::user_defaults_api::CUserDefaults::instance().getServerInfoFileLocationSrv());
        LOG(MiscCommon::info) << "Reading server info from: " << sSrvCfg;
        if (sSrvCfg.empty())
            throw std::runtime_error("Can't find server info file.");

        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(sSrvCfg, pt);
        *_host = pt.get<std::string>(_name + ".host");
        *_port = pt.get<std::string>(_name + ".port");
    }

    inline void findCommanderServer(std::string* _host, std::string* _port)
    {
        findCommanderServerImpl(_host, _port, "server");
    }

    inline void findCommanderUI(std::string* _host, std::string* _port)
    {
        findCommanderServerImpl(_host, _port, "ui");
    }
}; // namespace MiscCommon

#endif
