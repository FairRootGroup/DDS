// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// BOOST
#include <boost/program_options/cmdline.hpp>
//#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
// STD
#include <iostream>
#include <fstream>
#include <string>
// MiscCommon
#include "DDSUserDefaultsOptions.h"
#include "BOOSTHelper.h"
#include "SysHelper.h"
#include "Res.h"
//
#include "version.h"

using namespace DDS;
using namespace MiscCommon;
using namespace std;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;

void printVersion()
{
    cout << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
         << "DDS configuration"
         << " v" << USER_DEFAULTS_CFG_VERSION << "\n" << g_cszReportBugsAddr << endl;
}

// Command line parser
bool parseCmdLine(int _Argc, char* _Argv[], bool* _verbose) throw(exception)
{
    // Generic options
    bpo::options_description visible("Options");
    // WORKAROUND: repeat add_options call to help clang-format, otherwise it produce ureadable output
    visible.add_options()("help,h", "Produce help message");
    visible.add_options()("version,v", "Version information");
    visible.add_options()("path,p", "Show DDS user defaults config file path");
    /*    (
            "config,c", bpo::value<string>(), "DDS user defaults configuration file")("key", bpo::value<string>(), "Get a value for the given key")(
            "default,d", "Generate a default PoD configuration file")(
            "force,f", "If the destination file exists, remove it and create a new file, without prompting for confirmation")(
            "userenvscript", "Show the full path of user's environment script for workers (if present). The path must be evaluated before use")(
            "wrkpkg", "Show the full path of the worker package. The path must be evaluated before use")(
            "wrkscript", "Show the full path of the worker script. The path must be evaluated before use")(
            "wn-sandbox-dir", "Show the full path of the sandbox directory. The path must be evaluated before use")(
            "verbose,V", "Cause pod-user-defaults to be verbose in case of an error");
    */
    // Parsing command-line
    bpo::variables_map vm;
    bpo::store(bpo::command_line_parser(_Argc, _Argv).options(visible).run(), vm);
    bpo::notify(vm);

    if (vm.count("help") || vm.empty())
    {
        cout << visible << endl;
        return false;
    }
    if (vm.count("version"))
    {
        printVersion();
        return false;
    }
    *_verbose = vm.count("verbose");

    boost_hlp::option_dependency(vm, "default", "config");
    boost_hlp::conflicting_options(vm, "default", "key");
    boost_hlp::conflicting_options(vm, "force", "key");

    return true;
}

int main(int argc, char* argv[])
{
    // Command line parser
    bool verbose(false);
    try
    {
        if (!parseCmdLine(argc, argv, &verbose))
            return 0;
    }
    catch (exception& e)
    {
        if (verbose)
            cerr << e.what() << endl;
        return 1;
    }

    return 0;
}