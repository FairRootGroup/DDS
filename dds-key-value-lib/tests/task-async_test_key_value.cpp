// DDS
#include "KeyValue.h"
#include "Logger.h"
// STD
#include <vector>
#include <iostream>
#include <exception>
#include <sstream>
#include <condition_variable>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
namespace bpo = boost::program_options;
using namespace MiscCommon;

const size_t g_maxValue = 1000;
const size_t g_maxWaitTime = 100; // milliseconds

int main(int argc, char* argv[])
{
    try
    {
        string sKey;
        size_t nInstances(0);
        size_t nMaxValue(g_maxValue);
        size_t nMaxWaitTime(g_maxWaitTime);

        // Generic options
        bpo::options_description options("task-async_test_key_value options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("key", bpo::value<string>(&sKey)->default_value("property1"), "key to update");
        options.add_options()(
            "instances,i", bpo::value<size_t>(&nInstances)->default_value(0), "A number of instances");
        options.add_options()(
            "max-value", bpo::value<size_t>(&nMaxValue)->default_value(g_maxValue), "A max value of the property");
        options.add_options()("max-wait-time",
                              bpo::value<size_t>(&nMaxWaitTime)->default_value(g_maxWaitTime),
                              "A max wait time (in milliseconds), which an instannces should wait before exit");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return false;
        }

        // Named mutex
        char* ddsTaskId;
        ddsTaskId = getenv("DDS_TASK_ID");
        if (NULL == ddsTaskId)
            throw runtime_error("USER TASK: Can't initialize semaphore because DDS_TASK_ID variable is not set");
        const string taskID(ddsTaskId);

        // TODO: document the test workflow
        // The test workflow
        // #1.

        CKeyValue ddsKeyValue;
        mutex keyMutex;
        condition_variable keyCondition;

        bool bGoodToGo = false;

        // container
        typedef set<string /*prop values*/> val_t;
        typedef map<string /*propname*/, val_t> container_t;
        container_t valContainer;

        // Subscribe on key update events
        ddsKeyValue.subscribe(
            [&keyCondition, &keyMutex, &valContainer, &bGoodToGo, &nMaxValue](const string& _key, const string _value)
            {
                LOG(debug) << "USER TASK received key update notification";
                {
                    unique_lock<mutex> lk(keyMutex);
                    // Add new value

                    val_t* values = &valContainer[_key];
                    values->insert(_value);
                    // check wheather all values are already there
                    for (const auto& prop : valContainer)
                    {
                        bGoodToGo = (prop.second.size() == nMaxValue);
                        if (!bGoodToGo)
                            break;
                    }
                }
                keyCondition.notify_all();
            });

        for (size_t i = 0; i < nMaxValue; ++i)
        {
            LOG(debug) << "USER TASK is going to set new value " << i;
            const string sCurValue = to_string(i);
            LOG(debug) << "USER TASK put value return code: " << ddsKeyValue.putValue(sKey, sCurValue);
        }

        while (true)
        {
            unique_lock<mutex> lk(keyMutex);
            if (bGoodToGo)
                return 0;
            // wait for a key update event
            auto now = chrono::system_clock::now();
            int isTimeout = keyCondition.wait_until(lk, now + chrono::milliseconds(nMaxWaitTime)) == cv_status::timeout;
            if (bGoodToGo)
                return 0;
            if (isTimeout)
                return 1;
        }
    }
    catch (const exception& _e)
    {
        LOG(fatal) << "USER TASK Error: " << _e.what() << endl;
        return 1;
    }
    return 1;
}
