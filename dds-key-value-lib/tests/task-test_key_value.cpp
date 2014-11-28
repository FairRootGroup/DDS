// DDS
#include "KeyValue.h"
#include "BOOSTHelper.h"
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

int main(int argc, char* argv[])
{
    try
    {
        string sWriteKey;
        string sWriteValue;
        string sReadKey;

        // Generic options
        bpo::options_description options("task-test_key_value options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("write-key", bpo::value<std::string>(&sWriteKey), "Specefies the key to update");
        options.add_options()(
            "write-value", bpo::value<std::string>(&sWriteValue), "Specefies the new value of the given key");
        options.add_options()("read-key", bpo::value<std::string>(&sReadKey), "Specefies the key to read");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return false;
        }

        MiscCommon::BOOSTHelper::option_dependency(vm, "write-key", "write-value");

        if (vm.count("write-key") && vm.count("write-value"))
        {
            CKeyValue ddsKeyValue;
            ddsKeyValue.putValue(sWriteKey, sWriteValue);
            cout << "Update key and value with: <" << sWriteKey << ", " << sWriteValue << ">" << endl;
        }

        if (vm.count("read-key"))
        {
            CKeyValue ddsKeyValue;
            CKeyValue::valuesMap_t values;
            ddsKeyValue.getValues(sReadKey, &values);
            while (values.empty())
            {
                ddsKeyValue.waitForUpdate(chrono::seconds(120));

                ddsKeyValue.getValues(sReadKey, &values);
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
