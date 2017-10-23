// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "DDSHelper.h"
#include "ErrorCode.h"
#include "Options.h"
#include "SubmitChannel.h"
// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::submit_cmd;

//=============================================================================
int main(int argc, char* argv[])
{
    // Command line parser
    SOptions_t options;
    try
    {
        Logger::instance().init();                         // Initialize log
        dds::user_defaults_api::CUserDefaults::instance(); // Initialize user defaults

        vector<std::string> arguments(argv + 1, argv + argc);
        ostringstream ss;
        copy(arguments.begin(), arguments.end(), ostream_iterator<string>(ss, " "));
        LOG(info) << "Starting with arguments: " << ss.str();

        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;
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

    string sHost;
    string sPort;
    try
    {
        // We want to connect to commnader's UI channel
        findCommanderUI(&sHost, &sPort);
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        LOG(log_stdout) << g_cszDDSServerIsNotFound_StartIt;
        return EXIT_FAILURE;
    }

    try
    {
        LOG(log_stdout) << "Contacting DDS commander on " << sHost << ":" << sPort << " ...";

        boost::asio::io_service io_service;

        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(sHost, sPort);

        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        CSubmitChannel::connectionPtr_t client = CSubmitChannel::makeNew(io_service, 0);

        client->setCfgFile(options.m_sCfgFile);
        client->setRMSType(options.m_sRMS);
        client->setPath(options.m_sPath);
        client->setNumber(options.m_number);

        client->connect(iterator);

        io_service.run();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
