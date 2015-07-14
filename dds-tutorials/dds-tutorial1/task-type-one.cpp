// DDS
#include "KeyValue.h"
#include "Logger.h"
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
using namespace MiscCommon;

// IDs of the DDS properties.
// Must be the same as in the topology file.
const string TaskIndexPropertyName = "TaskIndexProperty";
const string ReplyPropertyName = "ReplyProperty";

int main(int argc, char* argv[])
{
    Logger::instance().init(); // Initialize log

    std::mutex keyMutex;
    std::condition_variable keyCondition;

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

        dds::CKeyValue ddsKeyValue;

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

        dds::CKeyValue::valuesMap_t values;
        ddsKeyValue.getValues(ReplyPropertyName, &values);
        // We expect to receive one property from server.
        while (values.size() == 0)
        {
            std::unique_lock<std::mutex> lock(keyMutex);
            keyCondition.wait_until(lock, std::chrono::system_clock::now() + chrono::milliseconds(1000));
            ddsKeyValue.getValues(ReplyPropertyName, &values);
        }

        // We are done with key-value propagation
        // ddsKeyValue.unsubscribe();
        // ddsKeyValue.unsubscribeError();

        // Emulate data procesing of the task
        for (size_t i = 0; i < 5; ++i)
        {
            LOG(info) << "Work in progress (" << i << "/5)\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        LOG(info) << "Task successfully done\n";
    }
    catch (exception& _e)
    {
        LOG(error) << "USER TASK Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
