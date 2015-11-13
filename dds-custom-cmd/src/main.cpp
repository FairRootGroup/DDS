// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "CustomCmd.h"
#include "UserDefaults.h"
#include "Logger.h"
#include "Options.h"
// STD
#include <mutex>
#include <condition_variable>

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::custom_cmd;
using namespace dds::user_defaults_api;

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
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    try
    {
        CCustomCmd ddsCustomCmd;
        mutex replyMutex;
        condition_variable replyCondition;

        ddsCustomCmd.subscribeReply([&replyCondition](const string& _msg)
                                    {
                                        cout << "Received reply message: " << _msg << endl;
                                        replyCondition.notify_all();
                                    });

        int result = ddsCustomCmd.sendCmd(options.m_sCmd, options.m_sCondition);

        if (result == 1)
        { // error
            cerr << "Error sending custom command" << endl;
            return EXIT_FAILURE;
        }

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
