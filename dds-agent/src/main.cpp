// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "version.h"
#include "Options.h"
#include "ErrorCode.h"
#include "AgentConnectionManager.h"
#include "Logger.h"

using namespace std;
using namespace MiscCommon;
using namespace dds;

int main(int argc, char* argv[])
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    // Command line parser
    SOptions_t options;
    try
    {
        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
        return EXIT_FAILURE;
    }

    // Checking for "start" option
    if (SOptions_t::cmd_start == options.m_Command)
    {
        try
        {
            boost::asio::io_service service;
            CAgentConnectionManager agent(options, service);
            agent.start();
        }
        catch (exception& e)
        {
            LOG(fatal) << e.what();
            return EXIT_FAILURE;
        }
        catch (...)
        {
            LOG(fatal) << "Unexpected Exception occurred.";
            return EXIT_FAILURE;
        }
    }

    LOG(info) << "DDS agent is Done. Bye, Bye!";

    return EXIT_SUCCESS;
}
