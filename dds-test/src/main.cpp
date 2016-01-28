// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Process.h"
#include "DDSHelper.h"
#include "INet.h"
#include "Options.h"
#include "SysHelper.h"
#include "TestChannel.h"
#include "UserDefaults.h"

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace test_cmd;
using namespace dds::user_defaults_api;
using boost::asio::ip::tcp;

//=============================================================================
int main(int argc, char* argv[])
{
    // Command line parser
    SOptions_t options;
    try
    {
        Logger::instance().init(); // Initialize log
        CUserDefaults::instance(); // Initialize user defaults

        vector<std::string> arguments(argv + 1, argv + argc);
        ostringstream ss;
        copy(arguments.begin(), arguments.end(), ostream_iterator<string>(ss, " "));
        LOG(info) << "Starting with arguments: " << ss.str();

        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
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
        boost::asio::io_service io_service;

        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(sHost, sPort);

        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        CTestChannel::connectionPtr_t client = CTestChannel::makeNew(io_service);
        client->setOptions(options);
        client->connect(iterator);

        io_service.run();
    }
    catch (exception& e)
    {
        LOG(error) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
