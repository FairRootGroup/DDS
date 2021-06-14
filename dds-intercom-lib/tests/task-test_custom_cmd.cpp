// DDS
#include "EnvProp.h"
#include "Intercom.h"
// STD
#include <chrono>
#include <condition_variable>
#include <iostream>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
namespace bpo = boost::program_options;

int main(int argc, char* argv[])
{
    try
    {
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        // Generic options
        bpo::options_description options("task-test_custom_cmd options");
        options.add_options()("help,h", "Produce help message");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help"))
        {
            cout << options << endl;
            return 0;
        }

        // DDS custom command API
        CIntercomService service;
        CCustomCmd customCmd(service);

        // Waiting on this condition
        mutex waitMutex;
        condition_variable waitCondition;

        const string ddsSessionId{ env_prop<EEnvProp::dds_session_id>() };

        // Subscribe on errors
        service.subscribeOnError(
            [](const EErrorCode _errorCode, const string& _errorMsg)
            { cerr << "Error received: error code: " << _errorCode << ", error message: " << _errorMsg << endl; });

        // Subscribe on custom commands
        customCmd.subscribe(
            [&customCmd, &waitCondition, &ddsSessionId](
                const string& _command, const string& _condition, uint64_t _senderId)
            {
                cerr << "Received custom command: " << _command << " condition: " << _condition
                     << " senderId: " << _senderId << endl;

                if (_command == "exit")
                {
                    waitCondition.notify_all();
                }
                else
                {
                    // Check if DDS session ID is correct
                    string reply{ (ddsSessionId == _command) ? "ok" : "error" };
                    customCmd.send(reply, to_string(_senderId));
                }
            });

        // Subscribe on reply from DDS commander server
        customCmd.subscribeOnReply([](const string& _msg) { cout << "Received reply message: " << _msg; });

        service.start();

        // Wait until "exit" is received
        std::unique_lock<std::mutex> lk(waitMutex);
        waitCondition.wait(lk);

        cout << "Task successfully done" << endl;

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::seconds>(t2 - t1).count();
        cout << "Calculation time: " << duration << " seconds" << endl;
    }
    catch (exception& _e)
    {
        cerr << "USER TASK Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
