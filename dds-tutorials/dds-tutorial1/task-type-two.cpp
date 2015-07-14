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
namespace bpo = boost::program_options;

// IDs of the DDS properties.
// Must be the same as in the topology file.
const string TaskIndexPropertyName = "TaskIndexProperty";
const string ReplyPropertyName = "ReplyProperty";

int main(int argc, char* argv[])
{
    try
    {
        size_t nInstances(0);

        // Generic options
        bpo::options_description options("task-type-two options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()(
            "instances,i", bpo::value<size_t>(&nInstances)->default_value(0), "A number of client instances");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return false;
        }

        dds::CKeyValue ddsKeyValue;
        std::mutex keyMutex;
        std::condition_variable keyCondition;

        // Subscribe to DDS key-value error events.
        // Whenever an error occurs lambda will be called.
        ddsKeyValue.subscribeError([](const string& _msg)
                                   {
                                       cerr << "DDS key-value error: " << _msg << endl;
                                   });

        // Subscribe on key update events
        ddsKeyValue.subscribe([&keyCondition](const string& /*_key*/, const string& /*_value*/)
                              {
                                  keyCondition.notify_all();
                              });

        // First get all task index properties
        dds::CKeyValue::valuesMap_t taskValues;
        ddsKeyValue.getValues(TaskIndexPropertyName, &taskValues);
        while (taskValues.size() != nInstances)
        {
            std::unique_lock<std::mutex> lock(keyMutex);
            keyCondition.wait_until(lock, std::chrono::system_clock::now() + chrono::milliseconds(1000));
            ddsKeyValue.getValues(TaskIndexPropertyName, &taskValues);
        }
        for (const auto& v : taskValues)
        {
            cout << "Received task index: " << v.first << " --> " << v.second << endl;
        }

        // We have received all properties.
        // Broadcast property to all clients.
        std::string value = to_string(taskValues.size());
        if (0 != ddsKeyValue.putValue(ReplyPropertyName, value))
        {
            cerr << "DDS key-value putValue failed: key=" << ReplyPropertyName << " value=" << value << endl;
        }

        // We are done with key-value propagation
        // ddsKeyValue.unsubscribe();
        // ddsKeyValue.unsubscribeError();

        // Emulate data procesing of the task
        for (size_t i = 0; i < 7; ++i)
        {
            cout << "Work in progress (" << i << "/7)\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
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
