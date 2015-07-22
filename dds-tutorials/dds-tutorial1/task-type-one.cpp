// DDS
#include "KeyValue.h"
// STD
#include <vector>
#include <iostream>
#include <exception>
#include <sstream>
#include <condition_variable>
#include <thread>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
using namespace dds::key_value_api;
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
            return false;
        }

        CKeyValue ddsKeyValue;

        // Subscribe to DDS key-value error events.
        // Whenever an error occurs lambda will be called.
        ddsKeyValue.subscribeError([](const string& _msg)
                                   {
                                       cerr << "DDS key-value error: " << _msg;
                                   });

        // Put task index property
        if (0 != ddsKeyValue.putValue(TaskIndexPropertyName, optTaskIndex))
        {
            cerr << "DDS key-value putValue failed: key=" << TaskIndexPropertyName << " value=" << optTaskIndex << endl;
        }

        // Subscribe on key update events
        ddsKeyValue.subscribe([&keyCondition](const string& /*_key*/, const string& /*_value*/)
                              {
                                  keyCondition.notify_all();
                              });

        CKeyValue::valuesMap_t values;
        ddsKeyValue.getValues(ReplyPropertyName, &values);
        // We expect to receive one property from server.
        while (values.size() == 0)
        {
            unique_lock<mutex> lock(keyMutex);
            keyCondition.wait_until(lock, chrono::system_clock::now() + chrono::milliseconds(1000));
            ddsKeyValue.getValues(ReplyPropertyName, &values);
        }

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
