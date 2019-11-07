// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDSOPTIONS_H
#define DDSOPTIONS_H
//=============================================================================
// STD
#include <stdexcept>
#include <string>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
// DDS
#include "Logger.h"
#include "Res.h"
#include "UserDefaults.h"
#include "version.h"
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    namespace agent_cmd
    {
        /// \brief dds-commander's container of options
        typedef struct SOptions
        {
            enum ECommands
            {
                cmd_unknown,
                cmd_start,
                cmd_clean
            };
            SOptions()
            {
            }

            static ECommands getCommandByName(const std::string& _name)
            {
                if ("start" == _name)
                    return cmd_start;
                else if ("clean" == _name)
                    return cmd_clean;

                return cmd_unknown;
            }

            ECommands m_Command{ cmd_start };
            size_t m_slots{ 0 };
        } SOptions_t;
        //=============================================================================
        inline void PrintVersion()
        {
            LOG(MiscCommon::log_stdout) << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
                                        << "DDS configuration"
                                        << " v" << USER_DEFAULTS_CFG_VERSION << "\n"
                                        << MiscCommon::g_cszReportBugsAddr;
        }
        //=============================================================================
        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-agent options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()("command",
                                  bpo::value<std::string>(),
                                  "The command is a name of dds-agent command."
                                  " Can be one of the following: start, clean.\n"
                                  "For user's convenience it is allowed to call dds-agent without \"--command\" option"
                                  " by just specifying the command name directly, like:\ndds-agent start.\n\n"
                                  "Commands:\n"
                                  "   start: \tStart dds-agent\n"
                                  "   clean: \tCleaning");
            options.add_options()(
                "slots,s", bpo::value<size_t>(&_options->m_slots), "Defines a number of task slots per agent.");

            //...positional
            bpo::positional_options_description pd;
            pd.add("command", 1);

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).positional(pd).run(), vm);
            bpo::notify(vm);

            if (vm.count("help") || vm.empty())
            {
                LOG(MiscCommon::log_stdout) << options;
                return false;
            }
            if (vm.count("version"))
            {
                PrintVersion();
                return false;
            }

            // Command
            if (vm.count("command"))
            {
                if (SOptions::cmd_unknown == SOptions::getCommandByName(vm["command"].as<std::string>()))
                {
                    LOG(MiscCommon::log_stderr)
                        << PROJECT_NAME << " error: unknown command: " << vm["command"].as<std::string>() << "\n\n"
                        << options;
                    return false;
                }
            }
            else
            {
                LOG(MiscCommon::log_stderr) << PROJECT_NAME << ": Nothing to do\n\n" << options;
                return false;
            }

            _options->m_Command = SOptions::getCommandByName(vm["command"].as<std::string>());

            return true;
        }
    } // namespace agent_cmd
} // namespace dds
#endif
