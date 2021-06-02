// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "DDSHelper.h"
#include "ErrorCode.h"
#include "Options.h"
#include "Tools.h"
#include "Res.h"
#include "Environment.h"
// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::submit_cmd;
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
        dds::misc::setupEnv(); // Setup environment

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

    // List all avbaliable plug-ins
    if (options.m_bListPlugins)
    {
        namespace fs = boost::filesystem;
        string pluginsRootDir = (options.m_sPath.empty())
                                    ? dds::user_defaults_api::CUserDefaults::instance().getPluginsRootDir()
                                    : options.m_sPath;
        fs::path someDir(pluginsRootDir);
        fs::directory_iterator end_iter;

        typedef std::multimap<std::time_t, fs::path> result_set_t;
        result_set_t result_set;

        if (fs::exists(someDir) && fs::is_directory(someDir))
        {
            cout << "Avaliable RMS plug-ins:\n";
            for (fs::directory_iterator dir_iter(someDir); dir_iter != end_iter; ++dir_iter)
            {
                if (fs::is_directory(dir_iter->status()))
                {
                    // The plug-ins have names like "dds-submit-xxx", where xxx is a plug-in name
                    vector<string> parts;
                    boost::split(parts, dir_iter->path().stem().string(), boost::is_any_of("-"));
                    if (parts.size() == 3)
                        cout << "\t" << parts[2] << "\n";
                }
            }
            cout << endl;
        }
        else
        {
            cout << "Directory " << someDir << " doesn't exist or is not a directory.\n";
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

        LOG(MiscCommon::log_stdout) << "Connection established.";
        LOG(MiscCommon::log_stdout) << "Requesting server to process job submission...";

        SSubmitRequest::request_t requestInfo;
        requestInfo.m_config = options.m_sCfgFile;
        requestInfo.m_rms = options.m_sRMS;
        requestInfo.m_instances = options.m_number;
        requestInfo.m_slots = options.m_slots;
        requestInfo.m_pluginPath = options.m_sPath;
        SSubmitRequest::ptr_t requestPtr = SSubmitRequest::makeRequest(requestInfo);

        requestPtr->setMessageCallback([&isFailed](const SMessageResponseData& _message) {
            LOG((_message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                << "Server reports: " << _message.m_msg;

            if (_message.m_severity == dds::intercom_api::EMsgSeverity::error)
                isFailed = true;
        });

        requestPtr->setDoneCallback([&session, &start, &isFailed]() {
            if (isFailed)
            {
                LOG(MiscCommon::log_stderr) << "Submission has Failed";
            }
            else
            {
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double, std::milli> elapsed = end - start;
                LOG(MiscCommon::log_stdout) << "Submission took: " << elapsed.count() << " ms\n";
            }

            session.unblockCurrentThread();
        });

        session.sendRequest<SSubmitRequest>(requestPtr);

        session.blockCurrentThread();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return (isFailed ? EXIT_FAILURE : EXIT_SUCCESS);
}
