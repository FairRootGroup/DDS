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

const size_t g_maxValue = 1000;

int main(int argc, char* argv[])
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    try
    {
        size_t nInstances(0);
        size_t nMaxValue(g_maxValue);
        size_t type(0);
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

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return false;
        }

        testErrors = vm.count("test-errors");

        const std::vector<std::string> propNames_0 = {
            "property_1", "property_2", "property_3", "property_4", "property_5"
        };

        const std::vector<std::string> propNames_1 = {
            "property_6", "property_7", "property_8", "property_9", "property_10"
        };

        dds::CKeyValue ddsKeyValue;
        std::mutex keyMutex;
        std::condition_variable keyCondition;

        if (testErrors)
        {
            ddsKeyValue.subscribeError([&keyCondition](const string& _msg)
                                       {
                                           LOG(error) << "Key-value error: " << _msg;
                                       });
        }

        LOG(info) << "Start task with type " << type;

        for (size_t i = 0; i < nMaxValue; ++i)
        {
            LOG(info) << "Start iteration " << i;

            // For tasks with type 0 we start with writing the properties.
            if ((i % 2 == 0 && type == 0) || (i % 2 == 1 && type == 1))
            {
                LOG(info) << "Iteration " << i << " start sending values.";

                string writePropValue = to_string(i);
                const auto& writePropNames = (type == 0) ? propNames_0 : propNames_1;
                for (const auto& prop : writePropNames)
                {
                    if (0 != ddsKeyValue.putValue(prop, writePropValue))
                    {
                        LOG(fatal) << "putValue failed.";
                    }
                }

                LOG(info) << "Iteration " << i << " all values have been sent.";

                // Writing non existing and readonly properties to test the errors
                if (testErrors)
                {
                    LOG(info) << "Iteration " << i << " sending wrong properties.";
                    ddsKeyValue.putValue("non_existing_property", "non_existing_property_name");
                    const auto& readonlyPropNames = (type == 0) ? propNames_1 : propNames_0;
                    for (const auto& prop : readonlyPropNames)
                    {
                        if (0 != ddsKeyValue.putValue(prop, writePropValue))
                        {
                            LOG(fatal) << "putValue failed.";
                        }
                    }
                }
            }
            // For tasks with type 1 we start with subscribtion to properties.
            else if ((i % 2 == 0 && type == 1) || (i % 2 == 1 && type == 0))
            {
                LOG(info) << "Iteration " << i << " subscribe on property updates.";

                // Subscribe on key update events
                ddsKeyValue.subscribe([&keyCondition](const string& /*_key*/, const string& /*_value*/)
                                      {
                                          keyCondition.notify_all();
                                      });

                const auto& readPropNames = (type == 0) ? propNames_1 : propNames_0;
                for (const auto& prop : readPropNames)
                {
                    dds::CKeyValue::valuesMap_t values;
                    ddsKeyValue.getValues(prop, &values);
                    string value = to_string(i);

                    size_t counter = 0;
                    for (const auto& v : values)
                    {
                        if (v.second == value)
                            counter++;
                    }

                    while (counter != nInstances)
                    {
                        std::unique_lock<std::mutex> lock(keyMutex);
                        keyCondition.wait_until(lock, std::chrono::system_clock::now() + chrono::milliseconds(1000));
                        ddsKeyValue.getValues(prop, &values);

                        counter = 0;
                        for (const auto& v : values)
                        {
                            if (v.second == value)
                                counter++;
                        }
                    }
                }

                LOG(info) << "Iteration " << i << " got all properties.";
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(30));

        LOG(info) << "Task successfully done";
    }
    catch (exception& _e)
    {
        LOG(fatal) << "USER TASK Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
