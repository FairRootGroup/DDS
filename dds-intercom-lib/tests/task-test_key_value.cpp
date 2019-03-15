// DDS
#include "Logger.h"
#include "dds_intercom.h"
// STD
#include <chrono>
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
using namespace dds::user_defaults_api;
namespace bpo = boost::program_options;
using namespace MiscCommon;

const size_t g_maxValue = 1000;
const size_t g_timeout = 10; // in seconds

string map_to_string(const map<std::string, string>& _map)
{
    stringstream ss;
    ss << "Map with number of elements: " << _map.size() << ". Elements:\n";
    for (const auto& v : _map)
    {
        ss << v.first << " --> " << v.second << "\n";
    }
    return ss.str();
}

int main(int argc, char* argv[])
{
    try
    {
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        CUserDefaults::instance(); // Initialize user defaults
        Logger::instance().init(); // Initialize log

        size_t nInstances(0);
        size_t nMaxValue(g_maxValue);
        size_t type(0);
        size_t timeout(g_timeout);
        bool testErrors(true);

        // Generic options
        bpo::options_description options("task-test_key_value options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()(
            "instances,i", bpo::value<size_t>(&nInstances)->default_value(0), "A number of instances");
        options.add_options()(
            "max-value", bpo::value<size_t>(&nMaxValue)->default_value(g_maxValue), "A max value of the property");
        options.add_options()("type,t", bpo::value<size_t>(&type)->default_value(0), "Type of task. Must be 0 or 1.");
        options.add_options()("test-errors",
                              "Indicates that taks will also put incorrect data and test the error messages.");
        options.add_options()("timeout",
                              bpo::value<size_t>(&timeout)->default_value(g_timeout),
                              "A max timeout for a task to get all properties.");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return 0;
        }

        testErrors = vm.count("test-errors");

        const vector<string> propNames_0 = { "property_1", "property_2", "property_3", "property_4", "property_5" };

        const vector<string> propNames_1 = { "property_6", "property_7", "property_8", "property_9", "property_10" };

        LOG(info) << "Start task with type " << type;

        CIntercomService service;
        CKeyValue keyValue(service);
        mutex keyMutex;
        condition_variable keyCondition;
        // on task done condition
        condition_variable onTaskDoneCondition;

        map<string, string> keyValueCache;
        size_t numWaits = 0;
        size_t numUpdateKeyValueCalls = 0;
        size_t currentIteration = (type == 0) ? 1 : 0;
        size_t numTaskDone = 0;

        // Subscribe on error events
        service.subscribeOnError([/*&keyCondition*/](EErrorCode _errorCode, const string& _msg) {
            LOG(error) << "Key-value error code: " << _errorCode << ", message: " << _msg;
        });

        // Subscribe on task done notifications
        service.subscribeOnTaskDone(
            [&numTaskDone, nInstances, &onTaskDoneCondition](uint64_t _taskID, uint32_t _exitCode) {
                ++numTaskDone;
                LOG(info) << "Task Done notification received for task " << _taskID << " with exit code " << _exitCode;
                // TODO: In order to properly account finished tasks, use taskID to get task's name
                if (numTaskDone >= nInstances)
                    onTaskDoneCondition.notify_all();
            });

        // Subscribe on key update events
        // DDS garantees that this callback function will not be called in parallel from multiple threads.
        // It is safe to update global data without locks inside the callback.
        keyValue.subscribe([&keyCondition, &currentIteration, &keyValueCache, &nInstances, &numUpdateKeyValueCalls](
                               const string& _propertyName, const string& _value, uint64_t _senderTaskID) {
            numUpdateKeyValueCalls++;

            string key = _propertyName + "." + to_string(_senderTaskID);
            keyValueCache[key] = _value;

            // Check that all values in the key-value cache have a correct value
            size_t counter = 0;
            string currentValue = to_string(currentIteration);
            for (const auto& v : keyValueCache)
            {
                if (v.second == currentValue)
                    counter++;
            }

            if (counter == nInstances * 5)
            {
                currentIteration += 2;
                keyCondition.notify_all();
            }
        });

        // Start listening to events we have subscribed on
        service.start();

        for (size_t i = 0; i < nMaxValue; ++i)
        {
            LOG(info) << "Start iteration " << i << ". Current value: " << currentIteration;

            // For tasks with type 0 we start with writing the properties.
            if ((i % 2 == 0 && type == 0) || (i % 2 == 1 && type == 1))
            {
                LOG(info) << "Iteration " << i << " start sending values.";

                string writePropValue = to_string(i);
                const auto& writePropNames = (type == 0) ? propNames_0 : propNames_1;
                for (const auto& prop : writePropNames)
                {
                    keyValue.putValue(prop, writePropValue);
                }

                LOG(info) << "Iteration " << i << " all values have been sent.";

                // Writing non existing and readonly properties to test the errors
                if (testErrors)
                {
                    LOG(info) << "Iteration " << i << " sending wrong properties.";
                    keyValue.putValue("non_existing_property", "non_existing_property_name");
                    const auto& readonlyPropNames = (type == 0) ? propNames_1 : propNames_0;
                    for (const auto& prop : readonlyPropNames)
                    {
                        keyValue.putValue(prop, writePropValue);
                    }
                }
            }
            // For tasks with type 1 we start with subscribtion to properties.
            else if ((i % 2 == 0 && type == 1) || (i % 2 == 1 && type == 0))
            {
                LOG(info) << "Iteration " << i << " subscribe on property updates. Current value: " << currentIteration;

                unique_lock<mutex> lock(keyMutex);
                bool waitStatus = keyCondition.wait_for(
                    lock, chrono::seconds(timeout), [&currentIteration, &i] { return currentIteration > i; });

                // Timeout waiting for property updates
                if (waitStatus == false)
                {
                    LOG(error) << "Iteration " << i << " timed out waiting for property updates.";
                    LOG(error) << "Number of key-value update calls: " << numUpdateKeyValueCalls
                               << "; currentIteration: " << currentIteration << "; numWaits: " << numWaits;

                    LOG(error) << "Key value cache.\n" << map_to_string(keyValueCache);

                    if (currentIteration == nMaxValue - 1)
                    {
                        LOG(warning) << "Some properties of the LAST iteration are missing.";
                    }
                    else
                    {
                        LOG(fatal) << "Task failed: timeout wait for property updates.";
                        return EXIT_FAILURE;
                    }
                }

                LOG(info) << "Iteration " << i << " got all properties. Current value: " << currentIteration;
            }
        }

        if ((nMaxValue % 2 == 0 && type == 0) || (nMaxValue % 2 == 1 && type == 1))
        {
            unique_lock<mutex> lock(keyMutex);
            bool waitStatus = onTaskDoneCondition.wait_for(
                lock, chrono::seconds(timeout), [&numTaskDone, &nInstances] { return numTaskDone >= nInstances; });
            if (waitStatus == false)
            {
                LOG(error) << "Task failed: Timed out on waiting Task Done.";
                LOG(error) << "Finished tasks: " << numTaskDone << " expected: " << nInstances;
                return EXIT_FAILURE;
            }
        }

        LOG(info) << "Key value cache.\n" << map_to_string(keyValueCache);
        LOG(info) << "Task successfully done";

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::seconds>(t2 - t1).count();
        LOG(info) << "Calculation time: " << duration << " seconds";
    }
    catch (exception& _e)
    {
        LOG(fatal) << "USER TASK Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
