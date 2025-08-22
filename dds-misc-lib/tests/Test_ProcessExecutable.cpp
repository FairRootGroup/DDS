// Copyright 2020 GSI, Inc. All rights reserved.
//
// Unit tests, supporting executable
//
// BOOST
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
// MiscCommon
#include "Process.h"
//=============================================================================
using namespace dds::misc;
using namespace std;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

#if __has_include(<boost/process/v1.hpp>)
namespace bp = boost::process::v1;
#else
namespace bp = boost::process;
#endif

using namespace boost::program_options;

int main(int argc, char* argv[])
{
    po::options_description desc;
    desc.add_options()("exit-code", value<int>())("wait", value<int>())("output-file", value<string>());

    variables_map vm;
    command_line_parser parser(argc, argv);
    store(parser.options(desc).allow_unregistered().run(), vm);
    notify(vm);

    std::chrono::seconds timeout(std::chrono::seconds(2));
    if (vm.count("exit-code"))
    {
        return vm["exit-code"].as<int>();
    }
    if (vm.count("wait"))
    {
        timeout = std::chrono::seconds(vm["wait"].as<int>());
    }
    if (vm.count("output-file"))
    {
        try
        {
            string output;
            stringstream ssCmd;
            ssCmd << bp::search_path("hostname").string() << " -f";
            execute(ssCmd.str(), timeout, &output);
            if (!output.empty())
            {
                ofstream f;
                f.open(vm["output-file"].as<string>());
                if (f.is_open())
                {
                    f << output;
                }
            }
        }
        catch (...)
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
