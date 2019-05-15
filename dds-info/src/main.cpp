// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "BOOSTHelper.h"
#include "DDSHelper.h"
#include "ErrorCode.h"
#include "InfoChannel.h"
#include "Process.h"
#include "SysHelper.h"
#include "UserDefaults.h"

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::info_cmd;
using namespace dds::user_defaults_api;
using boost::asio::ip::tcp;

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

    try
    {
        string sHost;
        string sPort;
        // We connect to UI commander channel.
        findCommanderUI(&sHost, &sPort);

        boost::asio::io_context io_context;

        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ip::tcp::resolver::query query(sHost, sPort);

        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        CInfoChannel::connectionPtr_t client = CInfoChannel::makeNew(io_context, 0);
        client->setOptions(options);
        client->connect(iterator);

        io_context.run();
    }
    catch (exception& e)
    {
        if (options.m_bNeedDDSStatus || options.m_nIdleAgentsCount > 0)
        {
            LOG(log_stdout_clean) << "DDS commander server is not running.";
        }
        LOG(error) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
