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
#include "version.h"
#include "Res.h"
#include "ProtocolCommands.h"
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
                cmd_stop
            };
            SOptions()
                : m_Command(cmd_start)
                , m_RMS(SSubmitCmd::SSH)
            {
            }

            static ECommands getCommandByName(const std::string& _name)
            {
                if ("start" == _name)
                    return cmd_start;
                if ("stop" == _name)
                    return cmd_stop;

                return cmd_unknown;
            }

            ECommands m_Command;
            std::string m_sTopoFile;
            bool m_needCommanderPid;
            SSubmitCmd::ERmsType m_RMS;
            std::string m_sSSHCfgFile;
        } SOptions_t;
        //=============================================================================
        inline void PrintVersion()
        {
            LOG(MiscCommon::log_stdout) << " v" << PROJECT_VERSION_STRING << "\n"
                                        << "DDS configuration"
                                        << " v" << USER_DEFAULTS_CFG_VERSION << "\n" << MiscCommon::g_cszReportBugsAddr;
        }
        //=============================================================================
        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options) throw(std::exception)
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
                " Can be one of the following: start, and stop.\n"
                "For user's convenience it is allowed to call dds-commander without \"--command\" option"
                " by just specifying the command name directly, like:\ndds-commander start or dds-commander stop.\n\n"
                "Commands:\n"
                "   start: \tStart dds-commander daemon\n"
                "   stop: \tStop dds-commander daemon\n");

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
                    LOG(MiscCommon::log_stderr) << "unknown command: " << vm["command"].as<std::string>() << "\n\n"
                                                << options;
                    return false;
                }
            }
            else
            {
                LOG(MiscCommon::log_stderr) << "Nothing to do\n\n" << options;
                return false;
            }

            _options->m_Command = SOptions::getCommandByName(vm["command"].as<std::string>());

            return true;
        }
    }
}
#endif
