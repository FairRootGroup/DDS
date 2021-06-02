// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Intercom.h"
#include "Logger.h"
#include "Options.h"
#include "UserDefaults.h"
#include "Environment.h"
// STD
#include <condition_variable>
#include <mutex>

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::intercom_api;
using namespace dds::user_defaults_api;

//=============================================================================
int main(int argc, char* argv[])
{
    // Command line parser
    dds::custom_cmd::SOptions_t options;

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
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    try
    {
        CIntercomService service;
        CCustomCmd customCmd(service);
        mutex replyMutex;
        condition_variable replyCondition;

        customCmd.subscribeOnReply([&replyCondition](const string& _msg) {
            cout << "Received reply message: " << _msg << endl;
            replyCondition.notify_all();
        });

        customCmd.send(options.m_sCmd, options.m_sCondition);

        // TODO: theoretically reply callback can be called before the next lines are executed.
        // In this case the program  will not exit.
        unique_lock<mutex> lock(replyMutex);
        replyCondition.wait(lock);
    }
    catch (exception& e)
    {
        LOG(error) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
