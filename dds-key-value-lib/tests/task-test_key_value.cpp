// DDS
#include "KeyValue.h"
// STD
#include <vector>
#include <iostream>
#include <exception>
#include <sstream>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
namespace bpo = boost::program_options;

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

        // The test workflow
        // #1. Update the given key with value X (starting from X=1)
        // #2. Wait until all other instances of the key also get value X
        // #3. Repeat the procedure with X = X + 1
        // -->> return success (0) when keys of all instances gets max value
        // -->> return failed (1) when one or more instances failed to updated in a given amount of time

        CKeyValue ddsKeyValue;
        size_t nCurValue = 0;
        while (true)
        {
            ++nCurValue;
            if (nCurValue > g_maxValue)
                return 0;

            const string sCurValue = to_string(nCurValue);
            ddsKeyValue.putValue(sKey, sCurValue);

            CKeyValue::valuesMap_t values;
            ddsKeyValue.getValues(sKey, &values);
            bool bGoodToGo = false;
            bool isTimeOut = false;
            while (values.empty() || !bGoodToGo)
            {

                CKeyValue::valuesMap_t::iterator it = values.begin();
                while (it != values.end())
                {
                    bGoodToGo = (it->second == sCurValue);
                    if (!bGoodToGo)
                        break;
                    ++it;
                }
                if (!bGoodToGo && isTimeOut)
                    return 1;

                isTimeOut = ddsKeyValue.waitForUpdate(chrono::milliseconds(nMaxWaitTime));

                ddsKeyValue.getValues(sKey, &values);
            }
        }
    }
    catch (exception& _e)
    {
        cerr << "Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
