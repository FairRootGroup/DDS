// DDS
#include "Logger.h"
#include "dds_env_prop.h"
#include "dds_intercom.h"
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
using namespace MiscCommon;

const size_t g_maxValue = 1000;

int main(int argc, char* argv[])
{
    try
    {
        string sKey;
        size_t nInstances(0);
        size_t nMaxValue(g_maxValue);

        // Generic options
        bpo::options_description options("task-async_test_key_value options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("key", bpo::value<string>(&sKey)->default_value("property1"), "key to update");
        options.add_options()(
            "instances,i", bpo::value<size_t>(&nInstances)->default_value(0), "A number of instances");
        options.add_options()(
            "max-value", bpo::value<size_t>(&nMaxValue)->default_value(g_maxValue), "A max value of the property");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return false;
        }

        CIntercomService service;
        CKeyValue keyValue(service);
        mutex keyMutex;
        condition_variable keyCondition;

        // key-value cache
        typedef set<string /*prop values*/> values_t;
        typedef map<string /*propname*/, values_t> container_t;
        container_t valuesCache;

        // Subscribe to DDS key-value error events.
        // Whenever an error occurs lambda will be called.
        service.subscribeOnError([](EErrorCode _errorCode, const string& _msg) {
            LOG(error) << "DDS key-value error code: " << _errorCode << ", message: " << _msg << endl;
        });

        // Subscribe on key update events
        keyValue.subscribe([&keyCondition, &keyMutex, &valuesCache, &nMaxValue](
            const string& _propertyID, const string& _key, const string _value) {
            LOG(debug) << "USER TASK received key update notification";

            values_t& values = valuesCache[_key];
            values.insert(_value);
            // check wheather all values are already there
            bool goodToGo(true);
            for (const auto& v : valuesCache)
            {
                // Not all values received
                if (v.second.size() != nMaxValue)
                {
                    goodToGo = false;
                    break;
                }
            }
            // All values received
            if (goodToGo)
            {
                keyCondition.notify_all();
            }
        });

        service.start();

        for (size_t i = 0; i < nMaxValue; ++i)
        {
            LOG(debug) << "USER TASK is going to set new value " << i;
            const string sCurValue = to_string(i);
            keyValue.putValue(sKey, sCurValue);
        }

        // Waiting for condition
        unique_lock<mutex> lock(keyMutex);
        keyCondition.wait(lock);
    }
    catch (const exception& _e)
    {
        LOG(log_stderr) << "USER TASK Error: " << _e.what() << endl;
        return 1;
    }
    return 0;
}
