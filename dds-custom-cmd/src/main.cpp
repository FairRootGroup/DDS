// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Intercom.h"
#include "Logger.h"
#include "MiscSetup.h"
#include "Options.h"
#include "UserDefaults.h"
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
    dds::custom_cmd::SOptions_t options;
    if (dds::misc::defaultExecSetup<dds::custom_cmd::SOptions_t>(
            argc, argv, &options, &dds::custom_cmd::ParseCmdLine) == EXIT_FAILURE)
        return EXIT_FAILURE;

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
