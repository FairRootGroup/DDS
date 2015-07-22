// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "version.h"
#include "AgentConnectionManager.h"
#include "Options.h"
#include "ErrorCode.h"
#include "INet.h"
#include "Logger.h"
#include "KeyValueGuard.h"

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::agent;
using namespace dds::user_defaults_api;

int main(int argc, char* argv[])
{
    // Command line parser
    dds::agent::SOptions_t options;
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
            shared_ptr<CAgentConnectionManager> agentptr =
                make_shared<dds::agent::CAgentConnectionManager>(options, io_service);
            agentptr->start();
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
            dds::key_value_api::CKeyValueGuard::clean();
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
