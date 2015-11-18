// DDS
#include "CustomCmd.h"
// STD
#include <iostream>
#include <exception>
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
using namespace custom_cmd;
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

        CCustomCmd ddsCustomCmd;

        // Subscribe on custom commands
        ddsCustomCmd.subscribeCmd([&ddsCustomCmd](const string& _command, const string& _condition, uint64_t _senderId)
                                  {
                                      cout << "Received custom command: " << _command << " condition: " << _condition
                                           << " senderId: " << _senderId << endl;
                                      if (_command == "please-reply")
                                      {
                                          string senderIdStr = to_string(_senderId);
                                          ddsCustomCmd.sendCmd("reply-to-" + senderIdStr, senderIdStr);
                                      }
                                      else if (_command == "please-reply-ui")
                                      {
                                          string senderIdStr = to_string(_senderId);
                                          ddsCustomCmd.sendCmd("reply-to-ui-" + senderIdStr, senderIdStr);
                                      }
                                  });

        // Subscribe on reply from DDS commander server
        ddsCustomCmd.subscribeReply([](const string& _msg)
                                    {
                                        cout << "Received reply message: " << _msg << endl;
                                    });

        // Emulate data procesing of the task
        const int n = 60;
        for (size_t i = 0; i < n; ++i)
        {
            cout << "Work in progress (" << i << "/" << n << ")\n";
            ddsCustomCmd.sendCmd("please-reply", "");
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