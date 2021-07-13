// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDSOPTIONS_H
#define DDSOPTIONS_H
//=============================================================================
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
// DDS
#include "ProtocolCommands.h"
#include "Res.h"
#include "SubmitCmd.h"
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    namespace commander_cmd
    {
        /// \brief dds-commander's container of options
        typedef struct SOptions
        {
            enum ECommands
            {
                cmd_unknown,
                cmd_start,
                cmd_stop,
                cmd_prep_session
            };
            SOptions()
                : m_Command(cmd_start)
                , m_sid(boost::uuids::nil_uuid())
            {
            }

            static ECommands getCommandByName(const std::string& _name)
            {
                if ("start" == _name)
                    return cmd_start;
                if ("stop" == _name)
                    return cmd_stop;
                if ("prep-session" == _name)
                    return cmd_prep_session;

                return cmd_unknown;
            }

            ECommands m_Command;
            std::string m_sTopoFile;
            std::string m_sRMS;
            std::string m_sCfgFile;
            boost::uuids::uuid m_sid;
        } SOptions_t;

        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-commander options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()(
                "command",
                bpo::value<std::string>(),
                "The command is a name of a dds-commander command."
                " Can be one of the following: start, stop, and prep-session.\n"
                "For user's convenience it is allowed to call dds-commander without \"--command\" option"
                " by just specifying the command name directly, like:\ndds-commander start or dds-commander stop.\n\n"
                "Commands:\n"
                "   start: \tStart dds-commander daemon\n"
                "   stop: \tStop dds-commander daemon\n"
                "   prep-session: \tPrepares a DDS session and returns new session ID\n");
            options.add_options()("session,s", bpo::value<std::string>(), "DDS Session ID");

            //...positional
            bpo::positional_options_description pd;
            pd.add("command", 1);

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).positional(pd).run(), vm);
            bpo::notify(vm);

            if (vm.count("help") || vm.empty())
            {
                LOG(dds::misc::log_stdout) << options;
                return false;
            }
            if (vm.count("version"))
            {
                LOG(dds::misc::log_stdout) << dds::misc::DDSVersionInfoString();
                return false;
            }

            // Command
            if (vm.count("command"))
            {
                if (SOptions::cmd_unknown == SOptions::getCommandByName(vm["command"].as<std::string>()))
                {
                    LOG(dds::misc::log_stderr) << "unknown command: " << vm["command"].as<std::string>() << "\n\n"
                                               << options;
                    return false;
                }
            }
            else
            {
                LOG(dds::misc::log_stderr) << "Nothing to do\n\n" << options;
                return false;
            }

            if (vm.count("session"))
            {
                _options->m_sid = boost::uuids::string_generator()(vm["session"].as<std::string>());
            }

            _options->m_Command = SOptions::getCommandByName(vm["command"].as<std::string>());

            return true;
        }
    } // namespace commander_cmd
} // namespace dds
#endif
