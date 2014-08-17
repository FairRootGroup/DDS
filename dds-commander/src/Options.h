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
#include "Logger.h"
#include "ProtocolCommands.h"
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    //=============================================================================
    // A custom streamer to help boost program options to convert string options to ERmsType
    inline std::istream& operator>>(std::istream& _in, SSubmitCmd::ERmsType& _rms)
    {
        std::string token;
        _in >> token;
        if (token == "ssh")
            _rms = SSubmitCmd::SSH;
        else
            throw bpo::invalid_option_value("Invalid RMS");
        return _in;
    }

    /// \brief dds-commander's container of options
    typedef struct SOptions
    {
        enum ECommands
        {
            cmd_unknown,
            cmd_start,
            cmd_stop,
            cmd_status,
            cmd_submit,
            cmd_getlog,
            cmd_test
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
            if ("status" == _name)
                return cmd_status;
            if ("submit" == _name)
                return cmd_submit;
            if ("getlog" == _name)
                return cmd_getlog;
            if ("test" == _name)
                return cmd_test;

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
        options.add_options()("topo,t",
                              bpo::value<std::string>(&_options->m_sTopoFile),
                              "A topology file. The option can only be used with the \"submit\" command");
        options.add_options()(
            "rms,r",
            bpo::value<SSubmitCmd::ERmsType>(&_options->m_RMS),
            "Resource Management System. The option can only be used with the \"submit\" command (default: ssh)");
        options.add_options()("ssh-rms-cfg",
                              bpo::value<std::string>(&_options->m_sSSHCfgFile),
                              "A DDS's ssh plug-in configuration file. The option can only be used "
                              "with the submit command when \'ssh\' is used as RMS");
        options.add_options()(
            "command",
            bpo::value<std::string>(),
            "The command is a name of dds-commander command."
            " Can be one of the following: start, stop, status, and info.\n"
            "For user's convenience it is allowed to call dds-commander without \"--command\" option"
            " by just specifying the command name directly, like:\ndds-commander start or dds-commander status.\n\n"
            "Commands:\n"
            "   start: \tStart dds-commander daemon\n"
            "   stop: \tStop dds-commander daemon\n"
            "   status: \tQuery current status of dds-command daemon\n"
            "   submit: \tSubmit a topology to a defined RMS\n"
            "   getlog: \tGet logs from agents.\n");

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

            if (SOptions::cmd_submit == SOptions::getCommandByName(vm["command"].as<std::string>()))
            {
                if (!vm.count("topo"))
                {
                    LOG(MiscCommon::log_stderr) << "specify a topo file"
                                                << "\n\n" << options;
                    return false;
                }

                if (_options->m_RMS == SSubmitCmd::SSH && !vm.count("ssh-rms-cfg"))
                {
                    LOG(MiscCommon::log_stderr) << "The SSH plug-in requires a rms configuration file. Please us "
                                                   "--ssh-rms-cfg to specify a desired configuration file."
                                                << "\n\n" << options;
                    return false;
                }
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
#endif
