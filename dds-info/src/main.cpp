// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "UserDefaults.h"
#include "version.h"
// BOOST
#include <boost/program_options/cmdline.hpp>
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
    visible.add_options()("verbose,V", "Cause pod-user-defaults to be verbose in case of an error");

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