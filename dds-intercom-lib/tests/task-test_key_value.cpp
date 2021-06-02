// DDS
#include "EnvProp.h"
#include "Intercom.h"
#include "Topology.h"
// STD
#include <chrono>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <map>
#include <sstream>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
using namespace dds::topology_api;
namespace bpo = boost::program_options;

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

        cout << "Start task with type " << type << endl;

        // Get list of READ and WRITE properties from topology
        CTopology topo;
        uint64_t taskId = env_prop<EEnvProp::task_id>();
        const STopoRuntimeTask& runtimeTask = topo.getRuntimeTaskById(taskId);
        const CTopoProperty::PtrMap_t& properties = runtimeTask.m_task->getProperties();
        vector<string> readPropertyNames;
        vector<string> writePropertyNames;
        for (auto property : properties)
        {
            CTopoProperty::EAccessType accessType = property.second->getAccessType();
            string propertyName(property.first);
            if (accessType == CTopoProperty::EAccessType::READ)
            {
                readPropertyNames.push_back(propertyName);
            }
            else
            {
                writePropertyNames.push_back(propertyName);
            }
        }

        // DDS Intercom API
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
            cerr << "Key-value error code: " << _errorCode << ", message: " << _msg << endl;
        });

        // Subscribe on task done notifications
        service.subscribeOnTaskDone([&numTaskDone, nInstances, &onTaskDoneCondition](uint64_t _taskID,
                                                                                     uint32_t _exitCode) {
            ++numTaskDone;
            cout << "Task Done notification received for task " << _taskID << " with exit code " << _exitCode << endl;
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
            cout << "Start iteration " << i << ". Current value: " << currentIteration << endl;

            // For tasks with type 0 we start with writing the properties.
            if ((i % 2 == 0 && type == 0) || (i % 2 == 1 && type == 1))
            {
                cout << "Iteration " << i << " start sending values." << endl;

                string writePropValue = to_string(i);
                for (const auto& prop : writePropertyNames)
                {
                    keyValue.putValue(prop, writePropValue);
                }

                cout << "Iteration " << i << " all values have been sent." << endl;

                // Writing non existing and readonly properties to test the errors
                if (testErrors)
                {
                    cout << "Iteration " << i << " sending wrong properties." << endl;
                    keyValue.putValue("non_existing_property", "non_existing_property_name");
                    for (const auto& prop : readPropertyNames)
                    {
                        keyValue.putValue(prop, writePropValue);
                    }
                }
            }
            // For tasks with type 1 we start with subscribtion to properties.
            else if ((i % 2 == 0 && type == 1) || (i % 2 == 1 && type == 0))
            {
                cout << "Iteration " << i << " subscribe on property updates. Current value: " << currentIteration
                     << endl;

                unique_lock<mutex> lock(keyMutex);
                bool waitStatus = keyCondition.wait_for(
                    lock, chrono::seconds(timeout), [&currentIteration, &i] { return currentIteration > i; });

                // Timeout waiting for property updates
                if (waitStatus == false)
                {
                    cerr << "Iteration " << i << " timed out waiting for property updates." << endl
                         << "Number of key-value update calls: " << numUpdateKeyValueCalls
                         << "; currentIteration: " << currentIteration << "; numWaits: " << numWaits << endl
                         << "Key value cache.\n"
                         << map_to_string(keyValueCache);

                    if (currentIteration == nMaxValue - 1)
                    {
                        cerr << "Some properties of the LAST iteration are missing." << endl;
                    }
                    else
                    {
                        cerr << "Task failed: timeout wait for property updates." << endl;
                        return EXIT_FAILURE;
                    }
                }

                cout << "Iteration " << i << " got all properties. Current value: " << currentIteration << endl;
            }
        }

        if ((nMaxValue % 2 == 0 && type == 0) || (nMaxValue % 2 == 1 && type == 1))
        {
            unique_lock<mutex> lock(keyMutex);
            bool waitStatus = onTaskDoneCondition.wait_for(
                lock, chrono::seconds(timeout), [&numTaskDone, &nInstances] { return numTaskDone >= nInstances; });
            if (waitStatus == false)
            {
                cerr << "Task failed: Timed out on waiting Task Done." << endl
                     << "Finished tasks: " << numTaskDone << " expected: " << nInstances;
                return EXIT_FAILURE;
            }
        }

        cout << "Key value cache.\n" << map_to_string(keyValueCache) << endl;
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
