// Copyright 2014 GSI, Inc. All rights reserved.
//
// DDS
#include "DDSHelper.h"
#include "ErrorCode.h"
#include "MiscSetup.h"
#include "Options.h"
#include "Res.h"
#include "Tools.h"
// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <iomanip>
#include <iostream>
#include <numeric>

using namespace std;
using namespace dds;
using namespace dds::submit_cmd;
using namespace dds::user_defaults_api;
using namespace dds::tools_api;
using namespace dds::misc;

// Define an enum for better type safety
enum class JobState : uint32_t
{
    PENDING = 0,
    RUNNING = 1,
    COMPLETED = 2,
    FAILED = 3,
    UNKNOWN = 4
};

string getStateString(JobState state)
{
    switch (state)
    {
        case JobState::PENDING:
            return "PENDING";
        case JobState::RUNNING:
            return "RUNNING";
        case JobState::COMPLETED:
            return "COMPLETED";
        case JobState::FAILED:
            return "FAILED";
        default:
            return "UNKNOWN";
    }
}

//=============================================================================
int main(int argc, char* argv[])
{
    SOptions_t options;
    if (dds::misc::defaultExecSetup<SOptions_t>(argc, argv, &options, &ParseCmdLine) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if (dds::misc::defaultExecReinit(options.m_sid) == EXIT_FAILURE)
        return EXIT_FAILURE;

    // List all available plug-ins
    if (options.m_bListPlugins)
    {
        namespace fs = boost::filesystem;
        string pluginsRootDir = (options.m_sPath.empty())
                                    ? dds::user_defaults_api::CUserDefaults::instance().getPluginsRootDir()
                                    : options.m_sPath;
        const fs::path pluginsDir(pluginsRootDir);

        if (fs::exists(pluginsDir) && fs::is_directory(pluginsDir))
        {
            cout << "Available RMS plug-ins:\n";
            for (const auto& entry : fs::directory_iterator(pluginsDir))
            {
                if (fs::is_directory(entry.status()))
                {
                    // The plug-ins have names like "dds-submit-xxx", where xxx is a plug-in name
                    vector<string> parts;
                    boost::split(parts, entry.path().stem().string(), boost::is_any_of("-"));
                    if (parts.size() == 3)
                        cout << "\t" << parts[2] << "\n";
                }
            }
            cout << endl;
        }
        else
        {
            cout << "Directory " << pluginsDir << " doesn't exist or is not a directory.\n";
        }
        return EXIT_SUCCESS;
    }

    string sid;
    try
    {
        sid = CUserDefaults::instance().getLockedSID();
    }
    catch (...)
    {
        LOG(log_stderr) << g_cszDDSServerIsNotFound_StartIt << endl;
        return EXIT_FAILURE;
    }

    std::atomic<bool> isFailed(false);
    try
    {
        std::chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
        CSession session;
        session.attach(sid);
        LOG(log_stdout) << "Connection established.";
        LOG(log_stdout) << "Requesting server to process job submission...";

        SSubmitRequest::request_t requestInfo;
        requestInfo.m_config = options.m_sCfgFile;
        requestInfo.m_rms = options.m_sRMS;
        requestInfo.m_instances = options.m_number;
        requestInfo.m_minInstances = options.m_minInstances;
        requestInfo.m_slots = options.m_slots;
        // Flags
        requestInfo.setFlag(dds::tools_api::SSubmitRequestData::ESubmitRequestFlags::enable_overbooking,
                            options.m_bEnableOverbooking);

        requestInfo.m_pluginPath = options.m_sPath;
        requestInfo.m_groupName = options.m_groupName;
        requestInfo.m_submissionTag = options.m_submissionTag;
        requestInfo.m_envCfgFilePath = options.m_envCfgFilePath;

        // Pre-allocate string capacity to avoid multiple reallocations
        if (!options.m_inlineConfig.empty())
        {
            requestInfo.m_inlineConfig.reserve(std::accumulate(options.m_inlineConfig.begin(),
                                                               options.m_inlineConfig.end(),
                                                               0,
                                                               [](size_t sum, const string& s)
                                                               { return sum + s.size() + 1; }));

            for (const auto& config : options.m_inlineConfig)
            {
                requestInfo.m_inlineConfig += config;
                requestInfo.m_inlineConfig += "\n";
            }
        }

        SSubmitRequest::ptr_t requestPtr = SSubmitRequest::makeRequest(requestInfo);

        requestPtr->setResponseCallback(
            [](const SSubmitResponseData& _response)
            {
                LOG(log_stdout) << "\nSubmission details:";
                LOG(log_stdout) << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";

                if (!_response.m_jobIDs.empty())
                {
                    LOG(log_stdout) << "Job IDs:";
                    for (const auto& id : _response.m_jobIDs)
                    {
                        LOG(log_stdout) << "  • " << id;
                    }
                }

                if (_response.m_jobInfoAvailable)
                {
                    LOG(log_stdout) << "Allocated nodes: " << _response.m_allocNodes;
                    LOG(log_stdout) << "State: " << getStateString(static_cast<JobState>(_response.m_state));
                }
                else
                {
                    LOG(log_stdout) << "Warning: Job information is not fully available";
                }
                LOG(log_stdout) << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
            });

        requestPtr->setMessageCallback(
            [&isFailed](const SMessageResponseData& _message)
            {
                LOG((_message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                    << "Server reports: " << _message.m_msg;

                if (_message.m_severity == dds::intercom_api::EMsgSeverity::error)
                    isFailed = true;
            });

        requestPtr->setDoneCallback(
            [&session, &start, &isFailed]()
            {
                if (isFailed)
                {
                    LOG(log_stderr) << "Submission has Failed";
                }
                else
                {
                    auto end = chrono::high_resolution_clock::now();
                    chrono::duration<double, std::milli> elapsed = end - start;
                    LOG(log_stdout) << "Submission took: " << elapsed.count() << " ms\n";
                }

                session.unblockCurrentThread();
            });

        session.sendRequest<SSubmitRequest>(requestPtr);
        session.blockCurrentThread();
    }
    catch (const exception& e)
    {
        LOG(log_stderr) << "Error: " << e.what();
        return EXIT_FAILURE;
    }

    return (isFailed ? EXIT_FAILURE : EXIT_SUCCESS);
}
