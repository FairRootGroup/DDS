// DDS
#include "dds_intercom.h"
// STD
#include <condition_variable>
#include <exception>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include <boost/program_options/options_description.hpp>
#pragma clang diagnostic pop

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
            return EXIT_SUCCESS;
        }

        CIntercomService service;
        CKeyValue keyValue(service);
        mutex keyMutex;
        condition_variable keyCondition;
        map<string, string> keyValueCache;

        // Subscribe to DDS key-value error events.
        // Whenever an error occurs lambda will be called.
        service.subscribeOnError([](EErrorCode _errorCode, const string& _msg) {
            cerr << "DDS key-value error code: " << _errorCode << ", message: " << _msg << endl;
        });

        // Subscribe on key update events
        // DDS garantees that this callback function will not be called in parallel from multiple threads.
        // It is safe to update global data without locks inside the callback.
        keyValue.subscribe([&keyCondition, &nInstances, &keyValueCache](
                               const string& _propertyID, const string& _value, uint64_t _senderTaskID) {
            cout << "Received key-value update: propertyID=" << _propertyID << " value=" << _value
                 << " senderTaskID=" << _senderTaskID << std::endl;
            string key = _propertyID + "." + to_string(_senderTaskID);
            keyValueCache[key] = _value;
            if (keyValueCache.size() == nInstances)
            {
                keyCondition.notify_all();
            }
        });

        service.start();

        // Wait for condition. We have to receive nInstances key-value updates.
        unique_lock<mutex> lock(keyMutex);
        keyCondition.wait(lock);

        // Print key-value cache
        for (const auto& v : keyValueCache)
        {
            cout << v.first << " --> " << v.second << std::endl;
        }

        // We have received all properties.
        // Broadcast property to all clients.
        string value = to_string(keyValueCache.size());
        keyValue.putValue(ReplyPropertyName, value);

        // Emulate data processing of the task
        for (size_t i = 0; i < 7; ++i)
        {
            cout << "Work in progress (" << i << "/7)\n";
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
