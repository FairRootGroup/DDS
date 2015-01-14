// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "ErrorCode.h"
#include "SubmitChannel.h"
#include "Options.h"
#include "DDSHelper.h"

using namespace std;
using namespace MiscCommon;
using namespace dds;

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
        LOG(log_stdout) << "Contacting DDS commander on " << sHost << ":" << sPort << " ...";

        boost::asio::io_service io_service;

        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(sHost, sPort);

        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        CSubmitChannel::connectionPtr_t client = CSubmitChannel::makeNew(io_service);

        client->setTopoFile(options.m_sTopoFile);
        client->setSSHCfgFile(options.m_sSSHCfgFile);
        client->setRMSTypeCode(options.m_RMS);

        client->connect(iterator);

        io_service.run();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
