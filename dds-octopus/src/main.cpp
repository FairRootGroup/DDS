// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Options.h"
#include "TestPing.h"

using namespace std;
using namespace dds;
using namespace dds_octopus;

//=============================================================================
int main(int argc, char* argv[])
{
    // Command line parser
    SOptions_t options;
    try
    {
        vector<std::string> arguments(argv + 1, argv + argc);
        ostringstream ss;
        copy(arguments.begin(), arguments.end(), ostream_iterator<string>(ss, " "));

        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;

        // =====================================================
        // Test Ping
        // =====================================================
        {
            CTestPing test_ping(options);
            test_ping.init();
            if (TS_OK != test_ping.execute(20))
                return 1;
        }
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
