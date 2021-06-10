// DDS
#include "Intercom.h"
// STD
#include <exception>
#include <iostream>
#include <sstream>
#include <thread>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
namespace bpo = boost::program_options;

int main(int argc, char* argv[])
{
    try
    {
        // Generic options
        bpo::options_description options("task-custom-cmd options");
        options.add_options()("help,h", "Produce help message");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        // DDS custom command API
        CIntercomService service;
        CCustomCmd customCmd(service);

        // Best practice is to subscribe on errors first, before doing any other function calls.
        // Otherwise there is a chance to miss some of the error messages from DDS.
        service.subscribeOnError([](const EErrorCode _errorCode, const string& _errorMsg) {
            cout << "Error received: error code: " << _errorCode << ", error message: " << _errorMsg << endl;
        });

        // Subscribe on custom commands
        customCmd.subscribe([&customCmd](const string& _command, const string& _condition, uint64_t _senderId) {
            cout << "Received custom command: " << _command << " condition: " << _condition
                 << " senderId: " << _senderId << endl;
            string senderIdStr = to_string(_senderId);
            customCmd.send("reply_" + senderIdStr, senderIdStr);
        });

        // Subscribe on reply from DDS commander server
        customCmd.subscribeOnReply([](const string& _msg) { cout << "Received reply message: " << _msg << endl; });

        service.start();

        // Emulate data procesing of the task
        const int n = 60;
        for (size_t i = 0; i < n; ++i)
        {
            cout << "Work in progress (" << i << "/" << n << ")\n";
            customCmd.send("please-reply", "");
            this_thread::sleep_for(chrono::seconds(5));
        }

        cout << "Task successfully done\n";
    }
    catch (exception& _e)
    {
        cerr << "USER TASK Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
