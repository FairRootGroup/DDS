// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "UserDefaults.h"
#include "version.h"
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
#include "BOOSTHelper.h"
#include "SysHelper.h"
#include "Res.h"

using namespace dds;
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
    visible.add_options()("verbose,V", "Cause pod-user-defaults to be verbose in case of an error");
    visible.add_options()("path,p", "Show DDS user defaults config file path");
    visible.add_options()("default,d", "Generate a default PoD configuration file");
    visible.add_options()("config,c", bpo::value<string>()->default_value("~/.DDS/DDS.cfg"), "DDS user defaults configuration file")(
        "key", bpo::value<string>(), "Get a value for the given key");
    visible.add_options()("force,f", "If the destination file exists, remove it and create a new file, without prompting for confirmation");
    /*    (
            "userenvscript", "Show the full path of user's environment script for workers (if present). The path must be evaluated before use")(
            "wrkpkg", "Show the full path of the worker package. The path must be evaluated before use")(
            "wrkscript", "Show the full path of the worker script. The path must be evaluated before use")(
            "wn-sandbox-dir", "Show the full path of the sandbox directory. The path must be evaluated before use")(

    */
    // Parsing command-line
    bpo::variables_map vm;
    bpo::store(bpo::command_line_parser(_Argc, _Argv).options(visible).run(), vm);
    bpo::notify(vm);

    if (_Argc < 2 || vm.count("help") || vm.empty())
    {
        cout << visible << endl;
        return false;
    }
    if (vm.count("version"))
    {
        printVersion();
        return false;
    }
    if (vm.count("verbose"))
    {
        *_verbose = true;
    }
    if (vm.count("path"))
    {
        CUserDefaults ud;
        cout << ud.currentUDFile() << endl;
        return true;
    }

    string sCfgFileName(vm["config"].as<string>());
    smart_path(&sCfgFileName);

    if (vm.count("default"))
    {
        cout << "Generating a default DDS configuration file..." << endl;

        if (MiscCommon::file_exists(sCfgFileName) && !vm.count("force"))
            throw runtime_error("Error: Destination file exists. Please use -f options to overwrite it.");

        if (sCfgFileName.empty())
            throw runtime_error("Error: Destination file name is empty. Please use -c options to define it.");

        ofstream f(sCfgFileName.c_str());
        if (!f.is_open())
        {
            string s("Can't open file ");
            s += sCfgFileName;
            s += " for writing.";
            throw runtime_error(s);
        }

        f << "# DDS user defaults\n"
          << "# version: " << USER_DEFAULTS_CFG_VERSION << "\n"
          << "#\n"
          << "# Please use DDS User's Manual to find out more details on\n"
          << "# keys and values of this configuration file.\n"
          << "# PoD User's Manual can be found in $DDS_LOCATION/doc folder or\n"
          << "# by the following address: http://dds.gsi.de/documentation.html\n";
        CUserDefaults::printDefaults(f);
        cout << "Generating a default DDS configuration file - DONE." << endl;
        return false;
    }

    // Check UD
    CUserDefaults user_defaults;
    try
    {
        user_defaults.init(sCfgFileName);
    }
    catch (std::exception& _e)
    {
        stringstream ss;
        ss << "DDS user defaults \"" << sCfgFileName << "\" is illformed: " << _e.what();
        throw runtime_error(ss.str());
    }

    if (vm.count("key"))
    {
        cout << user_defaults.getValueForKey(vm["key"].as<string>()) << endl;
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