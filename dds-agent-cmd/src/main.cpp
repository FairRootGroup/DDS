// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "DDSHelper.h"
#include "MiscSetup.h"
#include "Options.h"
#include "ProgressDisplay.h"
#include "Res.h"
#include "Tools.h"
#include "UserDefaults.h"

using namespace std;
using namespace dds;
using namespace dds::agent_cmd_cmd;
using namespace dds::user_defaults_api;
using namespace dds::tools_api;
using namespace dds::misc;

//=============================================================================
int main(int argc, char* argv[])
{
    SOptions_t options;
    if (dds::misc::defaultExecSetup<SOptions_t>(argc, argv, &options, &ParseCmdLine) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if (dds::misc::defaultExecReinit(options.m_sid) == EXIT_FAILURE)
        return EXIT_FAILURE;

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

        LOG(log_stdout) << "Connection established.";
        switch (options.m_agentCmd)
        {
            case EAgentCmdType::GETLOG:
                LOG(log_stdout) << "Requesting log files from agents...";
                break;
            default:
                LOG(log_stderr) << "Uknown command.";
                return EXIT_FAILURE;
        }

        SGetLogRequest::request_t requestInfo;
        SGetLogRequest::ptr_t requestPtr = SGetLogRequest::makeRequest(requestInfo);

        requestPtr->setMessageCallback(
            [&options](const SMessageResponseData& _message)
            {
                if (options.m_verbose || _message.m_severity == dds::intercom_api::EMsgSeverity::error)
                {
                    LOG((_message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                        << "Server reports: " << _message.m_msg;
                }
                else
                {
                    LOG((_message.m_severity == dds::intercom_api::EMsgSeverity::error) ? error : info)
                        << _message.m_msg;
                }
            });

        requestPtr->setDoneCallback([&session]() { session.unblockCurrentThread(); });

        requestPtr->setProgressCallback(
            [&options](const SProgressResponseData& _progress)
            {
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
