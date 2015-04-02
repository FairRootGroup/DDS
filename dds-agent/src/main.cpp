// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "version.h"
#include "Options.h"
#include "ErrorCode.h"
#include "AgentConnectionManager.h"
#include "INet.h"
#include "Logger.h"
#include "KeyValueGuard.h"

using namespace std;
using namespace MiscCommon;
using namespace dds;

int main(int argc, char* argv[])
{
    // Command line parser
    SOptions_t options;
    try
    {
        Logger::instance().init(); // Initialize log
        CUserDefaults::instance(); // Initialize user defaults

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
            boost::asio::io_service io_service;
            shared_ptr<CAgentConnectionManager> agent = make_shared<CAgentConnectionManager>(options, io_service);
            agent->start();
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

    // Checking for "Clean" option
    if (SOptions_t::cmd_clean == options.m_Command)
    {
        try
        {
            CKeyValueGuard::clean();
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
