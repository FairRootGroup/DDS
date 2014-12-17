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
        bpo::options_description options("task-test_key_value options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("key", bpo::value<std::string>(&sKey)->default_value("property1"), "key to update");
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

        // The test workflow
        // #1. Update the given key with value X (starting from X=1)
        // #2. Wait until all other instances of the key also get value X
        // #3. Repeat the procedure with X = X + 1
        // -->> return success (0) when keys of all instances gets max value
        // -->> return failed (1) when one or more instances failed to updated in a given amount of time

        CKeyValue ddsKeyValue;
        std::mutex keyMutex;
        std::condition_variable keyCondition;

        // Subscribe on key update events
        ddsKeyValue.subscribe([&keyCondition](const string& _key, const string _value)
                              {
                                  LOG(info) << "USER TASK received key update notification";
                                  keyCondition.notify_all();
                              });

        size_t nCurValue = 0;
        while (true)
        {
            ++nCurValue;
            if (nCurValue > g_maxValue)
                return 0;

            LOG(info) << "USER TASK is going to set new value " << nCurValue;
            const string sCurValue = to_string(nCurValue);
            const int retVal = ddsKeyValue.putValue(sKey, sCurValue);
            LOG(info) << "USER TASK put value return code: " << retVal;

            CKeyValue::valuesMap_t values;
            ddsKeyValue.getValues(sKey, &values);
            bool bGoodToGo = false;
            bool isTimeout = false;
            while (values.empty() || !bGoodToGo)
            {
                if (values.size() == nInstances)
                {
                    for (const auto& v : values)
                    {
                        // we should check against current and current+1 values, becasue
                        // of edge cases when key updates happen excatly before we try to read new values triggered by
                        // previouse update.
                        bGoodToGo = (v.second == to_string(nCurValue + 1) || v.second == sCurValue);
                        if (!bGoodToGo)
                            break;
                    }
                }

                if (bGoodToGo)
                    break;

                if (isTimeout)
                    return 1;

                // wait for a key update event
                auto now = std::chrono::system_clock::now();
                std::unique_lock<std::mutex> lk(keyMutex);
                isTimeout = keyCondition.wait_until(lk, now + chrono::milliseconds(nMaxWaitTime)) == cv_status::timeout;

                ddsKeyValue.getValues(sKey, &values);
            }
        }
    }
    catch (exception& _e)
    {
        LOG(fatal) << "USER TASK Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
