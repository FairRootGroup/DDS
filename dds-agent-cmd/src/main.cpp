// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "DDSHelper.h"
#include "Options.h"
#include "ProgressDisplay.h"
#include "Tools.h"
#include "UserDefaults.h"

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::agent_cmd_cmd;
using namespace dds::user_defaults_api;
using namespace dds::tools_api;

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

    try
    {
        CSession session;
        session.attach(sid);

        LOG(MiscCommon::log_stdout) << "Connection established.";
        switch (options.m_agentCmd)
        {
            case EAgentCmdType::GETLOG:
                LOG(MiscCommon::log_stdout) << "Requesting log files from agents...";
                break;
            default:
                LOG(MiscCommon::log_stderr) << "Uknown command.";
                return EXIT_FAILURE;
        }

        SGetLogRequest::request_t requestInfo;
        SGetLogRequest::ptr_t requestPtr = SGetLogRequest::makeRequest(requestInfo);

        requestPtr->setMessageCallback([&options](const SMessageResponseData& _message) {
            if (options.m_verbose || _message.m_severity == dds::intercom_api::EMsgSeverity::error)
            {
                LOG((_message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                    << "Server reports: " << _message.m_msg;
            }
            else
            {
                LOG((_message.m_severity == dds::intercom_api::EMsgSeverity::error) ? error : info) << _message.m_msg;
            }
        });

        requestPtr->setDoneCallback([&session]() { session.unblockCurrentThread(); });

        requestPtr->setProgressCallback([&options](const SProgressResponseData& _progress) {
            if (options.m_verbose)
                return;

            uint32_t completed = _progress.m_completed + _progress.m_errors;
            if (completed < _progress.m_total)
            {
                cout << getProgressDisplayString(completed, _progress.m_total);
                cout.flush();
            }
            else
            {
                cout << getProgressDisplayString(completed, _progress.m_total) << endl;
                cout << "Received: " << _progress.m_completed << " errors: " << _progress.m_errors
                     << " total: " << _progress.m_total << endl;
            }
        });

        session.sendRequest<SGetLogRequest>(requestPtr);

        session.blockCurrentThread();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
