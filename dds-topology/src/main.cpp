// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "ActivateChannel.h"
#include "DDSHelper.h"
#include "Options.h"
#include "Topology.h"
#include "UserDefaults.h"

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::topology_cmd;
using namespace dds::topology_api;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using boost::asio::ip::tcp;

//=============================================================================
int main(int argc, char* argv[])
{
    // Command line parser
    SOptions_t options;
    try
    {
        CUserDefaults::instance(); // Initialize user defaults
        Logger::instance().init(); // Initialize log

        vector<std::string> arguments(argv + 1, argv + argc);
        ostringstream ss;
        copy(arguments.begin(), arguments.end(), ostream_iterator<string>(ss, " "));
        LOG(info) << "Starting with arguments: " << ss.str();

        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;

        // Reinit UserDefaults and Log with new session ID
        CUserDefaults::instance().reinit(options.m_sid, CUserDefaults::instance().currentUDFile());
        Logger::instance().reinit();
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
        // We want to connect to commnader's UI channel
        findCommanderUI(&sHost, &sPort);
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
        if (options.m_topologyCmd == ETopologyCmdType::UPDATE || options.m_topologyCmd == ETopologyCmdType::ACTIVATE ||
            options.m_topologyCmd == ETopologyCmdType::STOP)
        {
            client = CActivateChannel::makeNew(io_service, 0);
            client->setOptions(options);
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
