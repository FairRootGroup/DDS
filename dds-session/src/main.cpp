// Copyright 2018 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "DDSHelper.h"
#include "ErrorCode.h"
#include "Options.h"
#include "Process.h"
// BOOST
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::submit_cmd;
using namespace dds::user_defaults_api;
namespace fs = boost::filesystem;

bool IsSessionRunning(const string& _sid)
{
    bool bRunning(false);

    CUserDefaults::instance().reinit(boost::uuids::string_generator()(_sid), CUserDefaults::instance().currentUDFile());

    fs::path pathPidFile(CUserDefaults::instance().getCommanderPidFile());

    if (!fs::is_regular_file(pathPidFile))
        return bRunning;

    pid_t pid(-1);
    ifstream f(pathPidFile.string());
    if (!f.is_open())
        return bRunning;
    f >> pid;

    bRunning = IsProcessRunning(pid);

    return bRunning;
}

//=============================================================================
int main(int argc, char* argv[])
{
    // Command line parser
    SOptions_t options;
    try
    {
        Logger::instance().init(); // Initialize log
        CUserDefaults::instance(); // Initialize user defaults

        vector<std::string> arguments(argv + 1, argv + argc);
        ostringstream ss;
        copy(arguments.begin(), arguments.end(), ostream_iterator<string>(ss, " "));
        LOG(info) << "Starting with arguments: " << ss.str();

        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;

        fs::path pathSessions(CUserDefaults::instance().getSessionsRootDir());
        pathSessions /= CUserDefaults::instance().getSessionsHolderDirName();

        vector<fs::path> session_dirs;
        vector<std::string> sessions;
        for (auto& dir : boost::make_iterator_range(fs::directory_iterator(pathSessions), {}))
        {
            session_dirs.push_back(dir.path());
            // Workaround: using .leaf().string(), instead of just .leaf(), vecasue we want to avoid double quotes
            // in output.
            sessions.push_back(dir.path().leaf().string());
        }

        if (!is_directory(pathSessions))
            return EXIT_SUCCESS;

        // ++++++++++++++++++
        // List All sessions
        // ++++++++++++++++++
        if (options.m_bListAll)
        {
            for (auto& dir : session_dirs)
            {
                string sSID = dir.leaf().string();
                time_t tmLastUseTime = fs::last_write_time(dir);
                char sLastUseTime[1000];
                struct tm* p = localtime(&tmLastUseTime);
                strftime(sLastUseTime, 1000, "%FT%TZ", p);

                cout << sSID << " \t [" << sLastUseTime << "] \t " << (IsSessionRunning(sSID) ? "RUNNING" : "STOPPED")
                     << endl;
            }
            return EXIT_SUCCESS;
        }
        // ++++++++++++++++++
        // Set default
        // ++++++++++++++++++
        if (!options.m_sDefault.empty())
        {
            auto found = find(std::begin(sessions), std::end(sessions), options.m_sDefault);
            if (found == std::end(sessions))
            {
                cerr << "Bad Session ID: " << options.m_sDefault << endl;
                return EXIT_FAILURE;
            }

            ofstream f(CUserDefaults::instance().getDefaultSIDFile());
            if (!f.is_open())
            {
                cerr << "Failed to set default session." << endl;
                return EXIT_FAILURE;
            }

            f << options.m_sDefault;
            f.close();
        }
        // ++++++++++++++++++
        // Remove All STOPPED sessions
        // ++++++++++++++++++
        if (options.m_bRemoveAllStopped)
        {
            for (auto& dir : session_dirs)
            {
                const string sSID(dir.leaf().string());

                if (IsSessionRunning(sSID))
                    continue;

                cout << "\n\nRemoving: " << sSID << endl;

                string sWrkDir(CUserDefaults::instance().getValueForKey("server.work_dir"));
                smart_path(&sWrkDir);
                string sLogDir(CUserDefaults::instance().getValueForKey("server.log_dir"));
                smart_path(&sLogDir);
                string sSandboxDir(CUserDefaults::instance().getValueForKey("server.server.sandbox_dir"));
                smart_path(&sSandboxDir);

                if (!options.m_bForce)
                {
                    cout << "\nThere following session directories will be removed." << endl;
                    if (!sWrkDir.empty())
                        cout << "DDS Work dir: " << sWrkDir << "\n";
                    if (!sLogDir.empty())
                        cout << "DDS Log dir: " << sLogDir << "\n";
                    if (!sSandboxDir.empty())
                        cout << "DDS Sandbox dir: " << sSandboxDir << "\n";

                    char answer(0);
                    do
                    {
                        cout << "Are you sure you want to proceed? [y/n]" << endl;
                        cin >> answer;
                    } while (!cin.fail() && answer != 'y' && answer != 'n');

                    if (answer == 'n')
                        return EXIT_SUCCESS;
                }

                if (!sWrkDir.empty())
                {
                    cout << "\tDDS Work dir: " << sWrkDir << "\n"
                         << "\tremoved files count: " << fs::remove_all(sWrkDir) << endl;
                }
                if (!sLogDir.empty())
                {
                    cout << "\tDDS Log dir: " << sLogDir << "\n"
                         << "\tremoved files count: " << fs::remove_all(sLogDir) << endl;
                }
                if (!sSandboxDir.empty())
                {
                    cout << "\tDDS Sandbox dir: " << sSandboxDir << "\n"
                         << "\tremoved files count: " << fs::remove_all(sSandboxDir) << endl;
                }
            }
            return EXIT_SUCCESS;
        }
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
