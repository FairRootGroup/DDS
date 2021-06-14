// DDS
#include "Intercom.h"
// STD
#include <atomic>
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
        std::string sessionID("");

        // Generic options
        bpo::options_description options("ui-custom-cmd options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("session,s", bpo::value<std::string>(&sessionID), "DDS Session ID");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (!vm.count("session"))
        {
            cout << "DDS session ID is not provided" << endl;
            return EXIT_SUCCESS;
        }

        atomic<size_t> counterCmdMessages(0);
        atomic<size_t> counterReplyMessages(0);

        // DDS custom command API
        CIntercomService service;
        CCustomCmd customCmd(service);

        // Best practice is to subscribe on errors first, before doing any other function calls.
        // Otherwise there is a chance to miss some of the error messages from DDS.
        service.subscribeOnError(
            [](const EErrorCode _errorCode, const string& _errorMsg)
            { cout << "Error received: error code: " << _errorCode << ", error message: " << _errorMsg << endl; });

        // Subscribe on custom commands
        customCmd.subscribe(
            [&counterCmdMessages](const string& _command, const string& _condition, uint64_t _senderId)
            {
                cout << "Received custom command " << counterCmdMessages << " : " << _command
                     << " condition: " << _condition << " senderId: " << _senderId << endl;
                counterCmdMessages++;
            });

        // Subscribe on reply from DDS commander server
        customCmd.subscribeOnReply(
            [&counterReplyMessages](const string& _msg)
            {
                cout << "Received reply message " << counterReplyMessages << " : " << _msg << endl;
                counterReplyMessages++;
            });

        service.start(sessionID);

        while (true)
        {
            customCmd.send("please-reply-ui", "main/GroupCustomCmd/TaskCustomCmd");
            this_thread::sleep_for(chrono::seconds(1));
            customCmd.send("please-reply-ui-regex", ".+Task.+");
            this_thread::sleep_for(chrono::seconds(1));
            customCmd.send("please-reply-ui-empty", "");
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
    catch (exception& _e)
    {
        cerr << "Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
