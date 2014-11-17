// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "UserDefaults.h"
#include "ActivateChannel.h"
#include "Options.h"
#include "DDSHelper.h"
#include "Topology.h"

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

    if (options.m_topologyCmd == ETopologyCmdType::VALIDATE)
    {
        try
        {
            CTopology topology;
            topology.init(options.m_sTopoFile);
        }
        catch (exception& e)
        {
            LOG(log_stderr) << e.what();
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    string sHost;
    string sPort;
    try
    {
        // Process server info file.
        findCommanderServer(&sHost, &sPort);
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        LOG(log_stdout) << g_cszDDSServerIsNotFound_StartIt;
        return EXIT_FAILURE;
    }

    try
    {
        LOG(log_stdout) << "Contacting DDS commander on " << sHost << ":" << sPort << "  ...";

        boost::asio::io_service io_service;

        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(sHost, sPort);

        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        CActivateChannel::connectionPtr_t client = nullptr;
        if (options.m_topologyCmd == ETopologyCmdType::ACTIVATE)
        {
            client = CActivateChannel::makeNew(io_service);
            client->connect(iterator);
        }

        io_service.run();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
