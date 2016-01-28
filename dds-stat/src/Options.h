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
#include "version.h"
// STD
#include <string>
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    namespace stat_cmd
    {
        /// \brief dds-commander's container of options
        typedef struct SOptions
        {
            SOptions()
                : m_bEnable(false)
                , m_bDisable(false)
                , m_bGet(false)
            {
            }

            bool m_bEnable;
            bool m_bDisable;
            bool m_bGet;
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
            bpo::options_description options("dds-stat options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()(
                "command",
                bpo::value<std::string>(),
                "The command is a name of a dds-stat command."
                " Can be one of the following: enable, disable and get.\n"
                "For user's convenience it is allowed to call dds-stat without \"--command\" option"
                " by just specifying the command name directly, like:\ndds-stat enable or dds-stat get.\n\n"
                "Commands:\n"
                "   enable: \tEnable statistics on the commander server.\n"
                "   disable: \tDisable statistics on the commander server.\n"
                "   get: \tGet statistics from the commander server.\n");

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

            if (vm.count("command"))
            {
                std::string cmd = vm["command"].as<std::string>();
                if (cmd == "enable")
                {
                    _options->m_bEnable = true;
                }
                else if (cmd == "disable")
                {
                    _options->m_bDisable = true;
                }
                else if (cmd == "get")
                {
                    _options->m_bGet = true;
                }
                else
                {
                    LOG(MiscCommon::log_stderr) << "unknown command: " << cmd << "\n\n" << options;
                    return false;
                }
            }
            else
            {
                LOG(MiscCommon::log_stderr) << "Nothing to do\n\n" << options;
                return false;
            }

            return true;
        }
    }
}
#endif
