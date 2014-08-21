// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Process.h"
#include "ErrorCode.h"
#include "BOOSTHelper.h"
#include "UserDefaults.h"
#include "SysHelper.h"
#include "SubmitChannel.h"
#include "Options.h"
#include "INet.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace MiscCommon;
using namespace dds;
using boost::asio::ip::tcp;

//=============================================================================
int main(int argc, char* argv[])
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    vector<std::string> arguments(argv + 1, argv + argc);
    ostringstream ss;
    copy(arguments.begin(), arguments.end(), ostream_iterator<string>(ss, " "));
    LOG(info) << "Starting with arguments: " << ss.str();

    // Command line parser
    SOptions_t options;
    try
    {
        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    try
    {
        // Read server info file
        const string sSrvCfg(CUserDefaults::instance().getServerInfoFileLocationSrv());
        LOG(info) << "Reading server info from: " << sSrvCfg;
        if (sSrvCfg.empty())
            throw runtime_error("Can't find server info file.");

        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(sSrvCfg, pt);
        const string sHost(pt.get<string>("server.host"));
        const string sPort(pt.get<string>("server.port"));

        LOG(log_stdout) << "Contacting DDS commander on " << sHost << ":" << sPort << " ...";

        boost::asio::io_service io_service;

        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(sHost, sPort);

        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        CSubmitChannel::connectionPtr_t client = CSubmitChannel::makeNew(io_service);
        client->setSendStart(options.m_bStart);
        client->connect(iterator);

        client->setTopoFile(options.m_sTopoFile);
        client->setSSHCfgFile(options.m_sSSHCfgFile);
        client->setRMSTypeCode(options.m_RMS);

        io_service.run();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
