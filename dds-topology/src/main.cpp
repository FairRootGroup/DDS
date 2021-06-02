// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "DDSHelper.h"
#include "Options.h"
#include "ProgressDisplay.h"
#include "Tools.h"
#include "TopoCore.h"
#include "UserDefaults.h"
#include "Res.h"
#include "MiscSetup.h"

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::topology_cmd;
using namespace dds::topology_api;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace dds::tools_api;

//=============================================================================
int main(int argc, char* argv[])
{
    SOptions_t options;
    if (dds::misc::defaultExecSetup<SOptions_t>(argc, argv, &options, &ParseCmdLine) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if (dds::misc::defaultExecReinit(options.m_sid) == EXIT_FAILURE)
        return EXIT_FAILURE;

    if (options.m_topologyCmd == ETopologyCmdType::VALIDATE)
    {
        try
        {
            CTopoCore topology;
            topology.init(options.m_sTopoFile);
        }
        catch (exception& e)
        {
            LOG(log_stderr) << e.what();
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (options.m_topologyCmd == ETopologyCmdType::REQUIRED_AGENTS)
    {
        try
        {
            CTopoCore topology;
            topology.setXMLValidationDisabled(options.m_bDisableValidation);
            topology.init(options.m_sTopoFile);
            auto v(topology.getRequiredNofAgents(10));
            LOG(log_stdout_clean) << v.first << " " << v.second;
        }
        catch (exception& e)
        {
            LOG(log_stderr) << e.what();
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (options.m_topologyCmd == ETopologyCmdType::TOPOLOGY_NAME)
    {
        try
        {
            CTopoCore topology;
            topology.setXMLValidationDisabled(options.m_bDisableValidation);
            topology.init(options.m_sTopoFile);
            LOG(log_stdout_clean) << topology.getName();
        }
        catch (exception& e)
        {
            LOG(log_stderr) << e.what();
            return EXIT_FAILURE;
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

    try
    {
        CSession session;
        session.attach(sid);

        LOG(MiscCommon::log_stdout) << "Connection established.";

        string action;
        switch (options.m_topologyCmd)
        {
            case ETopologyCmdType::ACTIVATE:
                action = "ACTIVATE";
                break;
            case ETopologyCmdType::STOP:
                action = "STOP";
                break;
            case ETopologyCmdType::UPDATE:
                action = "UPDATE";
                break;
            default:
                return EXIT_FAILURE;
        }

        LOG(MiscCommon::log_stdout) << "Requesting server to " << action << " the topology...";

        STopologyRequest::request_t topoInfo;

        switch (options.m_topologyCmd)
        {
            case ETopologyCmdType::ACTIVATE:
            case ETopologyCmdType::STOP:
            case ETopologyCmdType::UPDATE:
            {
                topoInfo.m_topologyFile = options.m_sTopoFile;
                topoInfo.m_disableValidation = options.m_bDisableValidation;
                // Set the proper update type
                if (options.m_topologyCmd == ETopologyCmdType::ACTIVATE)
                    topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
                else if (options.m_topologyCmd == ETopologyCmdType::UPDATE)
                    topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::UPDATE;
                else if (options.m_topologyCmd == ETopologyCmdType::STOP)
                    topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::STOP;
            }
            break;
            default:
                return EXIT_FAILURE;
        }

        STopologyRequest::ptr_t requestPtr = STopologyRequest::makeRequest(topoInfo);

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

                std::chrono::milliseconds timeToActivate(_progress.m_time);

                switch (_progress.m_srcCommand)
                {
                    case cmdACTIVATE_USER_TASK:
                        cout << "Activated tasks: " << _progress.m_completed << "\nErrors: " << _progress.m_errors
                             << "\nTotal: " << _progress.m_total
                             << "\nTime to Activate: " << std::chrono::duration<double>(timeToActivate).count() << " s"
                             << endl;
                        break;
                    case cmdASSIGN_USER_TASK:
                        cout << "Assigned/Uploaded tasks: " << _progress.m_completed
                             << "\nErrors: " << _progress.m_errors << "\nTotal: " << _progress.m_total
                             << "\nTime to assign/upload: " << std::chrono::duration<double>(timeToActivate).count()
                             << " s" << endl;
                        break;
                    case cmdSTOP_USER_TASK:
                        cout << "Stopped tasks: " << _progress.m_completed << "\nErrors: " << _progress.m_errors
                             << "\nTotal: " << _progress.m_total
                             << "\nTime to Stop: " << std::chrono::duration<double>(timeToActivate).count() << " s"
                             << endl;
                        break;
                    case cmdUPDATE_TOPOLOGY:
                        cout << "Updated agent topologies: " << _progress.m_completed
                             << "\nErrors: " << _progress.m_errors << "\nTotal: " << _progress.m_total
                             << "\nTime to update agent topologies: "
                             << std::chrono::duration<double>(timeToActivate).count() << " s" << endl;
                        break;
                    default:;
                }
            }
            return;
        });

        requestPtr->setDoneCallback([&session]() { session.unblockCurrentThread(); });

        session.sendRequest<STopologyRequest>(requestPtr);
        session.blockCurrentThread();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
