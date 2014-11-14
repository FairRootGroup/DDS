// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "UIConnectionManager.h"

using namespace dds;

CUIConnectionManager::CUIConnectionManager(boost::asio::io_service& _io_service,
                                           boost::asio::ip::tcp::endpoint& _endpoint)
    : CConnectionManagerImpl<CUIChannel, CUIConnectionManager>(_io_service, _endpoint)
{
}

CUIConnectionManager::~CUIConnectionManager()
{
}

void CUIConnectionManager::_createInfoFile(size_t _port) const
{
    const std::string sAgntCfg(CUserDefaults::instance().getAgentInfoFileLocation());
    LOG(MiscCommon::info) << "Creating the agent info file: " << sAgntCfg;
    std::ofstream f(sAgntCfg.c_str());
    if (!f.is_open() || !f.good())
    {
        std::string msg("Could not open the agent info configuration file: ");
        msg += sAgntCfg;
        throw std::runtime_error(msg);
    }

    std::string srvHost;
    MiscCommon::get_hostname(&srvHost);
    std::string srvUser;
    MiscCommon::get_cuser_name(&srvUser);

    f << "[agent]\n"
      << "host=" << srvHost << "\n"
      << "user=" << srvUser << "\n"
      << "port=" << _port << "\n" << std::endl;
}

void CUIConnectionManager::_deleteInfoFile() const
{
    const std::string sAgntCfg(CUserDefaults::instance().getAgentInfoFileLocation());
    if (sAgntCfg.empty())
        return;

    // TODO: check error code
    unlink(sAgntCfg.c_str());
}
