// DDS
#include "Intercom.h"
// STD
#include <condition_variable>
#include <exception>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
namespace bpo = boost::program_options;

// IDs of the DDS properties.
// Must be the same as in the topology file.
const string TaskIndexPropertyName = "TaskIndexProperty";
const string ReplyPropertyName = "ReplyProperty";

int main(int argc, char* argv[])
{
    mutex keyMutex;
    condition_variable keyCondition;

    try
    {
        string optTaskIndex;

        // Generic options
        bpo::options_description options("task-type-one options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("taskIndex", bpo::value<string>(&optTaskIndex), "task index");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return EXIT_SUCCESS;
        }

        CIntercomService service;
        CKeyValue keyValue(service);

        // Subscribe to DDS key-value error events.
        // Whenever an error occurs lambda will be called.
        service.subscribeOnError([](EErrorCode _errorCode, const string& _msg)
                                 { cerr << "DDS key-value error code: " << _errorCode << ", message: " << _msg; });

        // Subscribe on key update events
        keyValue.subscribe(
            [&keyCondition](const string& _propertyName, const string& _value, uint64_t _senderTaskID)
            {
                cout << "Received key-value update: propertyName=" << _propertyName << " value=" << _value
                     << " senderTaskId=" << _senderTaskID << std::endl;
                keyCondition.notify_all();
            });

        // Start listening to key-value updates
        service.start();

        // Put task index property
        keyValue.putValue(TaskIndexPropertyName, optTaskIndex);

        // Wait for condition. We have to receive only one key-value update.
        unique_lock<mutex> lock(keyMutex);
        keyCondition.wait(lock);

        // Emulate data procesing of the task
        for (size_t i = 0; i < 5; ++i)
        {
            cout << "Work in progress (" << i << "/5)\n";
            this_thread::sleep_for(chrono::seconds(1));
        }

        cout << "Task successfully done\n";
    }
    catch (exception& _e)
    {
        cerr << "USER TASK Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
