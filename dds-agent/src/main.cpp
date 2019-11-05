// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "AgentConnectionManager.h"
#include "ErrorCode.h"
#include "IntercomServiceCore.h"
#include "Logger.h"
#include "Options.h"
#include "SessionIDFile.h"
#include "UserDefaults.h"
#include "version.h"

// BOOST
#include <boost/interprocess/sync/named_mutex.hpp>

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::agent_cmd;
using namespace dds::user_defaults_api;

void clean()
{
    // Cleaning shared memory of agent's shared memory channel
    const CUserDefaults& userDefaults = CUserDefaults::instance();
    for (const auto& inputName : userDefaults.getSMLeaderInputNames())
    {
        const bool inputRemoved = boost::interprocess::message_queue::remove(inputName.c_str());
        LOG(MiscCommon::info) << "Message queue " << inputName << " remove status: " << inputRemoved;
    }

    boost::filesystem::path pathWrkDir(userDefaults.getSlotsRootDir());
    for (auto& dir : boost::make_iterator_range(boost::filesystem::directory_iterator(pathWrkDir), {}))
    {
        try
        {
            if (boost::filesystem::is_directory(dir.path()))
            {
                std::string filename = dir.path().filename().string();
                uint64_t slotID = std::stoull(filename);
                const std::string outputName(userDefaults.getSMLeaderOutputName(slotID));
                const bool outputRemoved = boost::interprocess::message_queue::remove(outputName.c_str());
                LOG(MiscCommon::info) << "Message queue " << outputName << " remove status: " << outputRemoved;
            }
        }
        catch (...)
        {
            continue;
        }
    }

    // Clean named mutex
    const string mutexName(userDefaults.getAgentNamedMutexName());
    const bool mutexRemoved = boost::interprocess::named_mutex::remove(mutexName.c_str());
    LOG(MiscCommon::info) << "Named mutex " << mutexName << " remove status: " << mutexRemoved;
}

int main(int argc, char* argv[])
{
    // Command line parser
    SOptions_t options;
    try
    {
        CUserDefaults::instance(); // Initialize user defaults
        Logger::instance().init(); // Initialize log

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
            CSessionIDFile sid(dds::user_defaults_api::CUserDefaults::instance().getSIDFile());
            LOG(info) << "SESSION ID file: " << dds::user_defaults_api::CUserDefaults::instance().getSIDFile();
            if (sid.getLockedSID().empty())
            {
                LOG(fatal) << "Failed to create session ID. Stopping the session...";
                return EXIT_FAILURE;
            }
            LOG(info) << "SESSION ID: " << sid.getLockedSID();

            // Export DDS session ID
            if (::setenv("DDS_SESSION_ID", sid.getLockedSID().c_str(), 1) == -1)
                throw MiscCommon::system_error("Failed to set up $DDS_SESSION_ID");

            shared_ptr<CAgentConnectionManager> agentptr = make_shared<CAgentConnectionManager>(options);
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
            dds::internal_api::CIntercomServiceCore::clean();
            clean();
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
