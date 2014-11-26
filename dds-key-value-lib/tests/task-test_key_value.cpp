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
        string key;
        string value;

        // Generic options
        bpo::options_description options("task-test_key_value options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("key", bpo::value<std::string>(&key), "Specefies the key to update");
        options.add_options()("value", bpo::value<std::string>(&value), "Specefies the new value of the given key");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return false;
        }

        MiscCommon::BOOSTHelper::option_dependency(vm, "key", "value");

        if (vm.count("key") && vm.count("value"))
        {
            CKeyValue ddsKeyValue;
            ddsKeyValue.putValue(key, value);
            cout << "Update key and value with: <" << key << ", " << value << ">" << endl;
        }
    }
    catch (exception& _e)
    {
        cerr << "Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
