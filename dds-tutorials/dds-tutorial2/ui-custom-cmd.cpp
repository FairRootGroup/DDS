// DDS
#include "dds_intercom.h"
// STD
#include <atomic>
#include <exception>
#include <iostream>
#include <sstream>
#include <thread>
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include <boost/program_options/options_description.hpp>
#pragma clang diagnostic pop

#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
namespace bpo = boost::program_options;

int main(int argc, char* argv[])
{
    try
    {
        // Generic options
        bpo::options_description options("ui-custom-cmd options");
        options.add_options()("help,h", "Produce help message");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        CCustomCmd customCmd;
        atomic<size_t> counterCmdMessages(0);
        atomic<size_t> counterReplyMessages(0);

        customCmd.subscribe(
            [&counterCmdMessages](const string& _command, const string& _condition, uint64_t _senderId) {
                cout << "Received custom command " << counterCmdMessages << " : " << _command
                     << " condition: " << _condition << " senderId: " << _senderId << endl;
                counterCmdMessages++;
            });

        customCmd.subscribeReply([&counterReplyMessages](const string& _msg) {
            cout << "Received reply message " << counterReplyMessages << " : " << _msg << endl;
            counterReplyMessages++;
        });

        while (true)
        {
            int result = customCmd.send("please-reply-ui", "");

            if (result == 1)
            {
                cerr << "Error sending custom command" << endl;
            }

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
